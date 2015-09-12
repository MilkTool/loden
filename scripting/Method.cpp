#include "Object.hpp"
#include "Collections.hpp"
#include "Method.hpp"
#include "StackInterpreter.hpp"

namespace Lodtalk
{

// CompiledMethod
Oop CompiledMethod::execute(Oop receiver, int argumentCount, Oop *arguments)
{
	return interpretCompiledMethod(this, receiver, argumentCount, arguments);
}

CompiledMethod *CompiledMethod::newMethodWithHeader(size_t numberOfBytes, CompiledMethodHeader header)
{
	// Add the method header size
	numberOfBytes += sizeof(void*);
	
	// Compute the some masks
	const size_t wordSize = sizeof(void*);
	const size_t wordMask = wordSize -1;
	
	// Compute the number of slots
	size_t slotCount = ((numberOfBytes + wordSize - 1) & (~wordMask)) / wordSize;
	size_t extraFormatBits = numberOfBytes & wordMask;
	assert(slotCount*wordSize >= numberOfBytes);
	
	// Compute the header size.
	auto headerSize = sizeof(ObjectHeader);
	if(slotCount >= 255)
		headerSize += 8;
	auto bodySize = slotCount*sizeof(void*);
	auto objectSize = headerSize + bodySize;
		
	// Allocate the compiled method
	auto methodData = allocateObjectMemory(objectSize);
	auto objectHeader = reinterpret_cast<ObjectHeader*> (methodData);
	
	// Set the method header
	objectHeader->slotCount = slotCount < 255 ? slotCount : 255;
	objectHeader->identityHash = generateIdentityHash(methodData);
	objectHeader->objectFormat = (unsigned int)(OF_COMPILED_METHOD + extraFormatBits);
	objectHeader->classIndex = SCI_CompiledMethod;
	if(slotCount >= 255)
	{
		auto bigHeader = reinterpret_cast<BigObjectHeader*> (objectHeader);
		bigHeader->slotCount = slotCount;
	}
	
	// Initialize the literals to nil
	auto methodBody = reinterpret_cast<Oop*> (methodData + headerSize);
	auto literalCount = header.getLiteralCount();
	for(size_t i = 0; i < literalCount + 1; ++i )
		methodBody[i] = nilOop();

	// Set the method header
	auto methodHeader = reinterpret_cast<CompiledMethodHeader*> (methodBody);
	*methodHeader = header;
	
	// Return the compiled method.
	return reinterpret_cast<CompiledMethod*> (methodData);
}

static Oop newMethodHeader(Oop clazz, Oop numberOfBytes, Oop headerWord)
{
	return nilOop();
} 

LODTALK_BEGIN_CLASS_SIDE_TABLE(CompiledMethod)
	LODTALK_METHOD("newMethod:header:", newMethodHeader)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(CompiledMethod)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(CompiledMethod, ByteArray, OF_COMPILED_METHOD, 0);

// NativeMethod
Oop NativeMethod::execute(Oop receiver, int argumentCount, Oop *arguments)
{
	return wrapper->execute(receiver, argumentCount, arguments);
}

LODTALK_BEGIN_CLASS_SIDE_TABLE(NativeMethod)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(NativeMethod)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(NativeMethod, Object, OF_INDEXABLE_8, sizeof(NativeMethodWrapper*));

// Native method descriptor
Oop NativeMethodDescriptor::getSelector() const
{
	selector = ByteSymbol::fromNative(selectorString).getOop();
	return selector.oop;
}

Oop NativeMethodDescriptor::getMethod() const
{
	auto data = allocateObjectMemory(sizeof(NativeMethod));
	nativeMethod.reset(new (data) NativeMethod(methodWrapper)); 
	
	return nativeMethod.getOop();
}

} // End of namespace Lodtalk
