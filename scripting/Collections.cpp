#include <string.h>
#include <map>
#include "Collections.hpp"

namespace Lodtalk
{
// Collection
LODTALK_BEGIN_CLASS_SIDE_TABLE(Collection)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(Collection)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(Collection, Object, OF_EMPTY, 0);

// SequenceableCollection
LODTALK_BEGIN_CLASS_SIDE_TABLE(SequenceableCollection)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(SequenceableCollection)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(SequenceableCollection, Collection, OF_EMPTY, 0);

// ArrayedCollection
LODTALK_BEGIN_CLASS_SIDE_TABLE(ArrayedCollection)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(ArrayedCollection)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(ArrayedCollection, SequenceableCollection, OF_EMPTY, 0);

// Array
Array *Array::basicNativeNew(size_t indexableSize)
{
	return reinterpret_cast<Array*> (newObject(0, indexableSize, OF_VARIABLE_SIZE_NO_IVARS, SCI_Array));
}

LODTALK_BEGIN_CLASS_SIDE_TABLE(Array)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(Array)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(Array, ArrayedCollection, OF_VARIABLE_SIZE_NO_IVARS, 0);

// ByteArray
LODTALK_BEGIN_CLASS_SIDE_TABLE(ByteArray)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(ByteArray)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(ByteArray, ArrayedCollection, OF_INDEXABLE_8, 0);

// FloatArray
LODTALK_BEGIN_CLASS_SIDE_TABLE(FloatArray)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(FloatArray)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(FloatArray, ArrayedCollection, OF_INDEXABLE_32, 0);

// WordArray
LODTALK_BEGIN_CLASS_SIDE_TABLE(WordArray)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(WordArray)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(WordArray, ArrayedCollection, OF_INDEXABLE_32, 0);

// IntegerArray
LODTALK_BEGIN_CLASS_SIDE_TABLE(IntegerArray)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(IntegerArray)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(IntegerArray, ArrayedCollection, OF_INDEXABLE_32, 0);

// String
LODTALK_BEGIN_CLASS_SIDE_TABLE(String)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(String)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(String, ArrayedCollection, OF_EMPTY, 0);

// ByteString
Ref<ByteString> ByteString::fromNative(const std::string &native)
{
	auto result = ClassObject->basicNativeNew(native.size());
	memcpy(result->getFirstFieldPointer(), native.data(), native.size());
	return Ref<ByteString> (reinterpret_cast<ByteString*> (result));
}

std::string ByteString::getString()
{
	auto begin = getFirstFieldPointer();
	return std::string(begin, begin + getNumberOfElements());
}

LODTALK_BEGIN_CLASS_SIDE_TABLE(ByteString)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(ByteString)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(ByteString, String, OF_INDEXABLE_8, 0);

// WideString
LODTALK_BEGIN_CLASS_SIDE_TABLE(WideString)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(WideString)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(WideString, String, OF_INDEXABLE_32, 0);

// Symbol
std::string ByteSymbol::getString()
{
	auto begin = getFirstFieldPointer();
	return std::string(begin, begin + getNumberOfElements());
}

LODTALK_BEGIN_CLASS_SIDE_TABLE(Symbol)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(Symbol)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(Symbol, String, OF_EMPTY, 0);

// ByteSymbol
LODTALK_BEGIN_CLASS_SIDE_TABLE(ByteSymbol)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(ByteSymbol)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(ByteSymbol, Symbol, OF_INDEXABLE_8, 0);

typedef std::map<std::string, Ref<ByteSymbol> > ByteSymbolDictionary;
static ByteSymbolDictionary *byteSymbolDictionary;

Object *ByteSymbol::basicNativeNew(size_t indexableSize)
{
	return reinterpret_cast<Object*> (newObject(0, indexableSize, OF_INDEXABLE_8, SCI_ByteSymbol));
}

Ref<ByteSymbol> ByteSymbol::fromNative(const std::string &native)
{
	if(!byteSymbolDictionary)
		byteSymbolDictionary = new ByteSymbolDictionary();
		
	// Find existing internation
	auto it = byteSymbolDictionary->find(native);
	if(it != byteSymbolDictionary->end())
		return it->second;

	// Create the byte symbol
	auto result = basicNativeNew(native.size());
	memcpy(result->getFirstFieldPointer(), native.data(), native.size());
	auto ref = Ref<ByteSymbol> (reinterpret_cast<ByteSymbol*> (result));
	
	// Store in the internation dictionary.
	(*byteSymbolDictionary)[native] = ref;
	return ref;
}

// WideSymbol
LODTALK_BEGIN_CLASS_SIDE_TABLE(WideSymbol)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(WideSymbol)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(WideSymbol, Symbol, OF_INDEXABLE_32, 0);

// HashedCollection
LODTALK_BEGIN_CLASS_SIDE_TABLE(HashedCollection)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(HashedCollection)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(HashedCollection, Collection, OF_FIXED_SIZE, 3);

// Dictionary
LODTALK_BEGIN_CLASS_SIDE_TABLE(Dictionary)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(Dictionary)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(Dictionary, HashedCollection, OF_FIXED_SIZE, 4);

// MethodDictionary
MethodDictionary* MethodDictionary::basicNativeNew()
{
	auto res = reinterpret_cast<MethodDictionary*> (newObject(4, 0, OF_FIXED_SIZE, SCI_MethodDictionary));
	res->capacityObject = Oop::encodeSmallInteger(0);
	res->tallyObject = Oop::encodeSmallInteger(0);
	return res;
}

LODTALK_BEGIN_CLASS_SIDE_TABLE(MethodDictionary)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(MethodDictionary)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(MethodDictionary, Dictionary, OF_FIXED_SIZE, 4);

// IdentityDictionary
LODTALK_BEGIN_CLASS_SIDE_TABLE(IdentityDictionary)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(IdentityDictionary)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(IdentityDictionary, Dictionary, OF_FIXED_SIZE, 3);

// SystemDictionary
LODTALK_BEGIN_CLASS_SIDE_TABLE(SystemDictionary)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(SystemDictionary)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(SystemDictionary, IdentityDictionary, OF_FIXED_SIZE, 3);

} // End of namespace Lodtalk
