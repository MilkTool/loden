#include <list>
#include <string.h>
#include "Common.hpp"
#include "ObjectModel.hpp"
#include "Object.hpp"
#include "Collections.hpp"
#include "Method.hpp"

#include "Compiler.hpp"
#include "InputOutput.hpp"
#include "MemoryManager.hpp"
#include "StackMemory.hpp"

namespace Lodtalk
{

static std::vector<Oop> classTable;
static std::vector<Oop> symbolTable;

// Special runtime objects
class SpecialRuntimeObjects
{
public:
	SpecialRuntimeObjects();
	~SpecialRuntimeObjects();

	void createSpecialObjectTable();
	void createSpecialClassTable();

	std::vector<Oop> specialObjectTable;
	std::vector<ClassDescription*> specialClassTable;

    size_t blockActivationSelectorFirst;
    size_t blockActivationSelectorCount;
    size_t specialMessageSelectorFirst;
    size_t specialMessageSelectorCount;
};

static SpecialRuntimeObjects *theSpecialRuntimeObjects = nullptr;
SpecialRuntimeObjects *getSpecialRuntimeObjects()
{
	if(!theSpecialRuntimeObjects)
		theSpecialRuntimeObjects = new SpecialRuntimeObjects();
	return theSpecialRuntimeObjects;
}

SpecialRuntimeObjects::SpecialRuntimeObjects()
{
	createSpecialObjectTable();
	createSpecialClassTable();
}

SpecialRuntimeObjects::~SpecialRuntimeObjects()
{
}

void SpecialRuntimeObjects::createSpecialObjectTable()
{
    WithoutGC wgc;

	specialObjectTable.push_back(nilOop());
	specialObjectTable.push_back(trueOop());
	specialObjectTable.push_back(falseOop());

    // Block activation selectors
    blockActivationSelectorFirst = specialObjectTable.size();
    specialObjectTable.push_back(makeSelector("value"));
    specialObjectTable.push_back(makeSelector("value:"));
    specialObjectTable.push_back(makeSelector("value:value:"));
    specialObjectTable.push_back(makeSelector("value:value:value:"));
    specialObjectTable.push_back(makeSelector("value:value:value:value:"));
    specialObjectTable.push_back(makeSelector("value:value:value:value:value:"));
    specialObjectTable.push_back(makeSelector("value:value:value:value:value:value:"));
    blockActivationSelectorCount = specialObjectTable.size() - blockActivationSelectorFirst;

    // Special message selectors
    specialMessageSelectorFirst = specialObjectTable.size();

    // Arithmetic messages
    specialObjectTable.push_back(makeSelector("+"));
    specialObjectTable.push_back(makeSelector("-"));
    specialObjectTable.push_back(makeSelector("<"));
    specialObjectTable.push_back(makeSelector(">"));
    specialObjectTable.push_back(makeSelector("<="));
    specialObjectTable.push_back(makeSelector(">="));
    specialObjectTable.push_back(makeSelector("="));
    specialObjectTable.push_back(makeSelector("~="));
    specialObjectTable.push_back(makeSelector("*"));
    specialObjectTable.push_back(makeSelector("/"));
    specialObjectTable.push_back(makeSelector("\\\\"));
    specialObjectTable.push_back(makeSelector("@"));
    specialObjectTable.push_back(makeSelector("bitShift:"));
    specialObjectTable.push_back(makeSelector("//"));
    specialObjectTable.push_back(makeSelector("bitAnd:"));
    specialObjectTable.push_back(makeSelector("bitOr:"));

    // Object accessing messages.
    specialObjectTable.push_back(makeSelector("at:"));
    specialObjectTable.push_back(makeSelector("at:put:"));
    specialObjectTable.push_back(makeSelector("size"));
    specialObjectTable.push_back(makeSelector("next"));
    specialObjectTable.push_back(makeSelector("nextPut:"));
    specialObjectTable.push_back(makeSelector("atEnd"));
    specialObjectTable.push_back(makeSelector("=="));
    specialObjectTable.push_back(makeSelector("class"));

    specialObjectTable.push_back(Oop()); // Unassigned

    // Block evaluation.
    specialObjectTable.push_back(makeSelector("value"));
    specialObjectTable.push_back(makeSelector("value:"));
    specialObjectTable.push_back(makeSelector("do:"));

    // Object instantiation.
    specialObjectTable.push_back(makeSelector("new"));
    specialObjectTable.push_back(makeSelector("new:"));
    specialObjectTable.push_back(makeSelector("x"));
    specialObjectTable.push_back(makeSelector("y"));

    specialMessageSelectorCount = specialObjectTable.size() - specialMessageSelectorFirst;
    assert(specialMessageSelectorCount == (size_t)SpecialMessageSelector::SpecialMessageCount);
}

void SpecialRuntimeObjects::createSpecialClassTable()
{
#define SPECIAL_CLASS_NAME(className) \
	specialClassTable.push_back(className::ClassObject); \
	specialClassTable.push_back(className::MetaclassObject);
#include "SpecialClasses.inc"
#undef SPECIAL_CLASS_NAME
}

void registerRuntimeGCRoots()
{
    auto specialObjects = getSpecialRuntimeObjects();


    // Register the runtime GC  objects table.
	registerGCRoot(&specialObjects->specialObjectTable[0], specialObjects->specialObjectTable.size());

	// Register the special classes table.
	registerGCRoot((Oop*)&specialObjects->specialClassTable[0], specialObjects->specialClassTable.size());

    // nil, true, false
    registerNativeObject(nilOop());
    registerNativeObject(trueOop());
    registerNativeObject(falseOop());

    // Special classes aren't created in the managed heap
    for(auto &specialClass : specialObjects->specialClassTable)
        registerNativeObject(Oop::fromPointer(specialClass));
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
Oop positiveInt32ObjectFor(uint32_t value)
{
    if(unsignedFitsInSmallInteger(value))
        return Oop::encodeSmallInteger(value);
    LODTALK_UNIMPLEMENTED();
}

Oop positiveInt64ObjectFor(uint64_t value)
{
    if(unsignedFitsInSmallInteger(value))
        return Oop::encodeSmallInteger(value);
    LODTALK_UNIMPLEMENTED();
}

Oop signedInt32ObjectFor(int32_t value)
{
    if(signedFitsInSmallInteger(value))
        return Oop::encodeSmallInteger(value);
    LODTALK_UNIMPLEMENTED();
}

Oop signedInt64ObjectFor(int64_t value)
{
    if(signedFitsInSmallInteger(value))
        return Oop::encodeSmallInteger(value);
    LODTALK_UNIMPLEMENTED();
}

uint32_t positiveInt32ValueOf(Oop value)
{
    if(value.isSmallInteger())
        return value.decodeSmallInteger();
    LODTALK_UNIMPLEMENTED();
}

uint64_t positiveInt64ValueOf(Oop value)
{
    if(value.isSmallInteger())
        return value.decodeSmallInteger();
    LODTALK_UNIMPLEMENTED();
}

Oop floatObjectFor(double value)
{
    LODTALK_UNIMPLEMENTED();
}

double floatValueOf(Oop object)
{
    LODTALK_UNIMPLEMENTED();
}

// Get a class from its index
Oop getClassFromIndex(int classIndex)
{
	// TODO: Use a proper class index manager
	auto specialObjects = getSpecialRuntimeObjects();
	if((size_t)classIndex >= specialObjects->specialClassTable.size())
	{
		LODTALK_UNIMPLEMENTED();
	}

	return Oop::fromPointer(specialObjects->specialClassTable[classIndex]);
}

Oop getClassFromOop(Oop oop)
{
	return getClassFromIndex(classIndexOf(oop));
}


bool isClassOrMetaclass(Oop oop)
{
	auto classIndex = classIndexOf(oop);
	if(classIndex == SCI_Metaclass)
		return true;

	auto clazz = getClassFromIndex(classIndexOf(oop));
	return classIndexOf(clazz) == SCI_Metaclass;
}

bool isMetaclass(Oop oop)
{
	return classIndexOf(oop) == SCI_Metaclass;
}

bool isClass(Oop oop)
{
	return isClassOrMetaclass(oop) && !isMetaclass(oop);
}

size_t getFixedSlotCountOfClass(Oop clazzOop)
{
    auto clazz = reinterpret_cast<ClassDescription*> (clazzOop.pointer);
    return clazz->fixedVariableCount.decodeSmallInteger();
}

size_t Oop::getFixedSlotCount() const
{
    return getFixedSlotCountOfClass(getClassFromOop(*this));
}

// Send the message
Oop sendDNUMessage(Oop receiver, Oop selector, int argumentCount, Oop *arguments)
{
	printf("TODO: Send DNU message\n");
	abort();
}

Oop lookupMessage(Oop receiver, Oop selector)
{
	auto classIndex = classIndexOf(receiver);
	auto classOop = getClassFromIndex(classIndex);
	assert(!isNil(classOop));

	// Lookup the method
	auto behavior = reinterpret_cast<Behavior*> (classOop.pointer);
	return behavior->lookupSelector(selector);
}

Oop sendMessage(Oop receiver, Oop selector, int argumentCount, Oop *arguments)
{
	auto method = lookupMessage(receiver, selector);
	if(isNil(method))
		return sendDNUMessage(receiver, selector, argumentCount, arguments);

	// Execute the method
	auto methodClassIndex = classIndexOf(method);
	if(methodClassIndex == SCI_CompiledMethod)
	{
		auto compiledMethod = reinterpret_cast<CompiledMethod*> (method.pointer);
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
static SystemDictionary *theGlobalDictionary = nullptr;
static SystemDictionary *getGlobalDictionary()
{
	if(!theGlobalDictionary)
	{
		theGlobalDictionary = SystemDictionary::create();
		registerGCRoot((Oop*)&theGlobalDictionary, 1);
	}

	return theGlobalDictionary;
}

Oop setGlobalVariable(const char *name, Oop value)
{
	return setGlobalVariable(ByteSymbol::fromNative(name).getOop(), value);
}

Oop setGlobalVariable(Oop symbol, Oop value)
{
	auto globalDictionary = getGlobalDictionary();

	// Set the existing value.
	auto globalVar = globalDictionary->getNativeAssociationOrNil(symbol);
	if(classIndexOf(Oop::fromPointer(globalVar)) == SCI_GlobalVariable)
	{
		globalVar->value = value;
		return Oop::fromPointer(globalVar);
	}

	// Create the global variable
	Ref<Association> newGlobalVar = GlobalVariable::make(symbol, value);
	globalDictionary->putNativeAssociation(newGlobalVar.get());
	return newGlobalVar.getOop();
}

Oop getGlobalFromName(const char *name)
{
	return getGlobalFromName(name);
}

Oop getGlobalFromSymbol(Oop symbol)
{
	return Oop::fromPointer(getGlobalDictionary()->getNativeAssociationOrNil(symbol));
}

Oop getGlobalValueFromName(const char *name)
{
	return getGlobalValueFromSymbol(ByteSymbol::fromNative(name).getOop());
}

Oop getGlobalValueFromSymbol(Oop symbol)
{
	auto globalDictionary = getGlobalDictionary();
	auto globalVar = globalDictionary->getNativeAssociationOrNil(symbol);
	if(classIndexOf(Oop::fromPointer(globalVar)) != SCI_GlobalVariable)
		return nilOop();
	return globalVar->value;
}

Oop getGlobalContext()
{
	return Oop::fromPointer(GlobalContext::ClassObject);
}

// Object reading
std::string getClassNameOfObject(Oop object)
{
	auto classIndex = classIndexOf(object);
	auto classOop = getClassFromIndex(classIndex);
	if(classOop.header->classIndex == SCI_Metaclass)
		return "a Class";

	auto clazz = reinterpret_cast<Class*> (classOop.pointer);
	return clazz->getNameString();
}

std::string getByteSymbolData(Oop object)
{
	auto symbol = reinterpret_cast<ByteSymbol*> (object.pointer);
	return symbol->getString();
}

std::string getByteStringData(Oop object)
{
	auto string = reinterpret_cast<ByteString*> (object.pointer);
	return string->getString();
}

// Special selectors.
Oop getBlockActivationSelector(size_t argumentCount)
{
    auto specialObjects = getSpecialRuntimeObjects();
    if(argumentCount >= specialObjects->blockActivationSelectorCount)
        return Oop();
    return specialObjects->specialObjectTable[specialObjects->blockActivationSelectorFirst + argumentCount];
}

Oop getSpecialMessageSelector(SpecialMessageSelector selectorIndex)
{
    auto specialObjects = getSpecialRuntimeObjects();
    return specialObjects->specialObjectTable[specialObjects->specialMessageSelectorFirst + (size_t)selectorIndex];
}

} // End of namespace Lodtalk
