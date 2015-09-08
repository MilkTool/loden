#include "Object.hpp"
#include "Collections.hpp"
#include "Method.hpp"

namespace Lodtalk
{

// CompiledMethod
Oop CompiledMethod::execute(Oop receiver, int argumentCount, Oop *arguments)
{
	printf("TODO: Implement compiled method execution\n");
	abort();
}

LODTALK_BEGIN_CLASS_SIDE_TABLE(CompiledMethod)
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
	return ByteSymbol::fromNative(selectorString).getOop();
}

Oop NativeMethodDescriptor::getMethod() const
{
	auto data = allocateObjectMemory(sizeof(NativeMethod));
	auto method = new (data) NativeMethod(methodWrapper); 
	return Oop::fromPointer(method);
}

} // End of namespace Lodtalk
