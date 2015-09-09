#include <vector>
#include <string.h>
#include "ObjectModel.hpp"
#include "Object.hpp"
#include "Collections.hpp"
#include "Method.hpp"

#include "InputOutput.hpp"

namespace Lodtalk
{

static std::vector<Oop> classTable;
static std::vector<Oop> symbolTable;

// Special objects
static const Oop specialObjectTableData[] = {
	nilOop(),
};

const Oop *specialObjectTable = specialObjectTableData;

// Special classes
static ClassDescription *specialClassTableData[] = {
#define SPECIAL_CLASS_NAME(className) \
	className::ClassObject, \
	className::MetaclassObject,
#include "SpecialClasses.inc"
#undef SPECIAL_CLASS_NAME
};

ClassDescription ** const specialClassTable = specialClassTableData;

uint8_t *allocateObjectMemory(size_t objectSize)
{
	// TODO: use a proper GCed heap.
	return (uint8_t*)malloc(objectSize);
}

ObjectHeader *newObject(size_t fixedSlotCount, size_t indexableSize, ObjectFormat format, int classIndex, int identityHash)
{
	// Compute the header size.
	size_t indexableSlotCount = 0;
	auto indexableSlotSize = variableSlotSizeFor(format);
	auto indexableFormatExtraBits = 0;
	bool hasPrimitiveData = false;
	if(indexableSlotSize && indexableSize)
	{
		if(format < OF_INDEXABLE_NATIVE_FIRST)
		{
			indexableSlotCount = indexableSize;
		}
#ifndef OBJECT_MODEL_SPUR_64
		if(format == OF_INDEXABLE_64)
		{
			indexableSlotCount = indexableSize * 2;
			hasPrimitiveData = true;
		}
#endif
		else
		{
			hasPrimitiveData = true;
			size_t divisor = variableSlotDivisor(format);
			size_t mask = (divisor - 1);

		 	indexableSlotCount = ((indexableSize + divisor - 1) & (~mask)) / divisor;
			indexableFormatExtraBits = indexableSize & mask;
			assert(indexableSize <= indexableSlotCount*divisor);
		}
	}
	
	// Compute more sizes
	auto totalSlotCount = fixedSlotCount + indexableSlotCount;
	auto headerSize = sizeof(ObjectHeader);
	if(totalSlotCount >= 255)
		headerSize += 8;
	auto fixedSlotSize = fixedSlotCount * sizeof(void*);
	auto variableSlotSize = indexableSlotCount * sizeof(void*);
	auto bodySize = fixedSlotSize + variableSlotSize;
	auto objectSize = headerSize + bodySize;

	// Allocate the object memory
	auto data = allocateObjectMemory(objectSize);
	auto header = reinterpret_cast<ObjectHeader*> (data);

	// Generate a hash if requested. 
	if(identityHash < 0)
		identityHash = generateIdentityHash(data);

	// Set the object header.
	*header = {0};
	header->slotCount = totalSlotCount < 255 ? totalSlotCount : 255;
	header->identityHash = identityHash;
	header->objectFormat = format + indexableFormatExtraBits;
	header->classIndex = classIndex;
	if(totalSlotCount >= 255)
	{
		auto bigHeader = reinterpret_cast<BigObjectHeader*> (header);
		bigHeader->slotCount = totalSlotCount;
	}
	
	// Initialize the slots.
	auto slotStarts = data + headerSize;
	if(!hasPrimitiveData)
	{
		auto slots = reinterpret_cast<Object**> (slotStarts);
		for(size_t i = 0; i < totalSlotCount; ++i)
			slots[i] = &NilObject;
	}
	else
	{
		memset(slotStarts, 0, bodySize);
	}

	// Return the object.
	return header;
}

// Object creation / accessing
Ref<ProtoObject> makeIntegerObject(int value)
{
	// TODO: Check the small integer range
	return Ref<ProtoObject>::fromOop(Oop::encodeSmallInteger(value));
}

Ref<ProtoObject> makeFloatObject(double value)
{
	// TODO: Implement this
	return makeRef(&NilObject);
}

// Get a class from its index
Oop getClassFromIndex(int classIndex)
{
	return Oop::fromPointer(specialClassTableData[classIndex]);
}

// Send the message
Oop sendDNUMessage(Oop receiver, Oop selector, int argumentCount, Oop *arguments)
{
	printf("TODO: Send DNU message\n");
	abort();
}

Oop sendMessage(Oop receiver, Oop selector, int argumentCount, Oop *arguments)
{
	auto classIndex = classIndexOf(receiver);
	auto classOop = getClassFromIndex(classIndex);
	assert(!isNil(classOop));
	
	// Lookup the method
	auto behavior = reinterpret_cast<Behavior*> (classOop.pointer);
	auto method = behavior->lookupSelector(selector);
	if(isNil(method))
		return sendDNUMessage(receiver, selector, argumentCount, arguments);
		
	// Execute the method
	auto methodClassIndex = classIndexOf(method);
	if(methodClassIndex == SCI_CompiledMethod)
	{
		auto compiledMethod = reinterpret_cast<NativeMethod*> (method.pointer);
		return compiledMethod->execute(receiver, argumentCount, arguments);
	}
	else if(methodClassIndex == SCI_NativeMethod)
	{
		auto nativeMethod = reinterpret_cast<NativeMethod*> (method.pointer);
		return nativeMethod->execute(receiver, argumentCount, arguments);
	}
	else
	{
		printf("TODO: Send run:with:in:\n");
		abort();
	}
}

Oop makeByteString(const std::string &content)
{
	return ByteString::fromNative(content).getOop();
}

Oop makeByteSymbol(const std::string &content)
{
	return ByteSymbol::fromNative(content).getOop();
}

Oop makeSelector(const std::string &content)
{
	return makeByteSymbol(content);
}

Oop sendBasicNew(Oop clazz)
{
	return sendMessage(clazz, makeSelector("basicNew"), 0, nullptr);
}

Oop sendBasicNewWithSize(Oop clazz, size_t size)
{
	Oop sizeOop = Oop::encodeSmallInteger(size);
	return sendMessage(clazz, makeSelector("basicNew:"), 1, &sizeOop);
}

// Global dictionary
static SystemDictionary *globalDictionary = nullptr;
 
Oop setGlobalVariable(const char *name, Oop value)
{
	return setGlobalVariable(ByteSymbol::fromNative(name).getOop(), value);
}

Oop setGlobalVariable(Oop symbol, Oop value)
{
	if(!globalDictionary)
		globalDictionary = new SystemDictionary();
		
	// Set the existing value.
	auto globalVar = globalDictionary->getNativeAssociationOrNil(symbol);
	if(classIndexOf(Oop::fromPointer(globalVar)) == SCI_GlobalVariable)
	{
		globalVar->value = value;
		return Oop::fromPointer(globalVar);
	}
	
	// Create the global variable
	globalVar = GlobalVariable::make(symbol, value);
	globalDictionary->putNativeAssociation(globalVar);
	return Oop::fromPointer(globalVar);
}

Oop getGlobalFromName(const char *name)
{
	return getGlobalFromName(name);
}

Oop getGlobalFromSymbol(Oop symbol)
{
	return Oop::fromPointer(globalDictionary->getNativeAssociationOrNil(symbol));
}

Oop getGlobalValueFromName(const char *name)
{
	return getGlobalValueFromSymbol(ByteSymbol::fromNative(name).getOop());
}

Oop getGlobalValueFromSymbol(Oop symbol)
{
	auto globalVar = globalDictionary->getNativeAssociationOrNil(symbol);
	if(classIndexOf(Oop::fromPointer(globalVar)) != SCI_GlobalVariable)
		return nilOop();
	return globalVar->value;
}

} // End of namespace Lodtalk
