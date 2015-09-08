#ifndef LODTALK_COLLECTIONS_HPP_
#define LODTALK_COLLECTIONS_HPP_

#include <stddef.h>
#include <string>
#include "Object.hpp"

namespace Lodtalk
{

/**
 * Collection
 */
class Collection: public Object
{
	LODTALK_NATIVE_CLASS();
public:
};

/**
 * SequenceableCollection
 */
class SequenceableCollection: public Collection
{
	LODTALK_NATIVE_CLASS();
public:
};

/**
 * ArrayedCollection
 */
class ArrayedCollection: public SequenceableCollection
{
	LODTALK_NATIVE_CLASS();
public:
};

/**
 * Array
 */
class Array: public ArrayedCollection
{
	LODTALK_NATIVE_CLASS();
public:
	static Array *basicNativeNew(size_t indexableSize);
};

/**
 * ByteArray
 */
class ByteArray: public ArrayedCollection
{
	LODTALK_NATIVE_CLASS();
public:
};

/**
 * FloatArray
 */
class FloatArray: public ArrayedCollection
{
	LODTALK_NATIVE_CLASS();
public:
};

/**
 * WordArray
 */
class WordArray: public ArrayedCollection
{
	LODTALK_NATIVE_CLASS();
public:
};

/**
 * IntegerArray
 */
class IntegerArray: public ArrayedCollection
{
	LODTALK_NATIVE_CLASS();
public:
};

/**
 * String
 */
class String: public ArrayedCollection
{
	LODTALK_NATIVE_CLASS();
public:
};

/**
 * ByteString
 */
class ByteString: public String
{
	LODTALK_NATIVE_CLASS();
public:

	static Ref<ByteString> fromNative(const std::string &native);
};

/**
 * WideString
 */
class WideString: public String
{
	LODTALK_NATIVE_CLASS();
public:
};

/**
 * Symbol
 */
class Symbol: public String
{
	LODTALK_NATIVE_CLASS();
public:
};

/**
 * ByteSymbol
 */
class ByteSymbol: public Symbol
{
	LODTALK_NATIVE_CLASS();
public:
	static Object *basicNativeNew(size_t indexableSize);
	
	static Ref<ByteSymbol> fromNative(const std::string &native);
};

/**
 * WideSymbol
 */
class WideSymbol: public Symbol
{
	LODTALK_NATIVE_CLASS();
public:
};

/**
 * HashedCollection
 */
class HashedCollection: public Collection
{
	LODTALK_NATIVE_CLASS();
public:
	HashedCollection()
	{
		capacityObject = Oop::encodeSmallInteger(0);
		tallyObject = Oop::encodeSmallInteger(0);
		keys = (Array*)&NilObject;
	}

	size_t getCapacity() const
	{
		return capacityObject.decodeSmallInteger();
	}

	size_t getTally() const
	{
		return tallyObject.decodeSmallInteger();
	}
	
	Oop *getHashTableKeys() const
	{
		assert(keys);
		return reinterpret_cast<Oop*> (keys->getFirstFieldPointer());
	}
	
protected:
	void setKeyCapacity(size_t keyCapacity)
	{
		keys = Array::basicNativeNew(keyCapacity);
	}

	template<typename HF, typename EF>
	ptrdiff_t findKeyPosition(Oop key, const HF &hashFunction, const EF &equals)
	{
		auto capacity = getCapacity();
		if(capacity == 0)
			return -1;

		auto keyHash = hashFunction(key);
		auto startPosition = keyHash % capacity;
		auto keyArray = getHashTableKeys();
		
		// Search from the hash position to the end.
		for(auto i = startPosition; i < capacity; ++i)
		{
			auto slotKey = keyArray[i];
			if(slotKey == nilOop() || equals(key, slotKey))
				return i;
		}

		// Search from the start to the hash position.
		for(auto i = 0; i < startPosition; ++i)
		{
			auto slotKey = keyArray[i];
			if(slotKey == nilOop() || equals(key, slotKey))
				return i;
		}
		
		// Not found.
		return -1;
	}
	
	Oop capacityObject;
	Oop tallyObject;
	Array* keys;
};

/**
 * Dictionary
 */
class Dictionary: public HashedCollection
{
	LODTALK_NATIVE_CLASS();
public:
	Dictionary()
	{
		values = (Array*)&NilObject;
	}

	Oop *getHashTableValues() const
	{
		assert(values);
		return reinterpret_cast<Oop*> (values->getFirstFieldPointer());
	}

protected:
	template<typename HF, typename EF>
	void increaseCapacity(const HF &hashFunction, const EF &equalityFunction)
	{
		size_t newCapacity = getCapacity()*2;
		if(!newCapacity)
			newCapacity = 16;
			
		setCapacity(newCapacity, hashFunction, equalityFunction);
	}

	template<typename HF, typename EF>
	Oop internalAtOrNil(Oop key, const HF &hashFunction, const EF &equalityFunction)
	{
		// If a slot was not found, try to increase the capacity.
		auto position = findKeyPosition(key, hashFunction, equalityFunction);
		if(position < 0)
			return nilOop();
			
		auto oldKey = getHashTableKeys()[position];
		if(isNil(oldKey))
			return nilOop();
		return getHashTableValues()[position];
	}

	template<typename HF, typename EF>
	void internalAtPut(Oop key, Oop value, const HF &hashFunction, const EF &equalityFunction)
	{
		// If a slot was not found, try to increase the capacity.
		auto position = findKeyPosition(key, hashFunction, equalityFunction);
		if(position < 0)
		{
			increaseCapacity(hashFunction, equalityFunction);
			return internalAtPut(key, value, hashFunction, equalityFunction);
		}
		
		// Put the key and value.
		auto keyArray = getHashTableKeys();
		auto valueArray = getHashTableValues();
		auto oldKey = keyArray[position];
		keyArray[position] = key;
		valueArray[position] = value;
		
		// Increase the size.
		if(oldKey == nilOop())
		{
			tallyObject.intValue += 2;
			// TODO: Check the increase capacity condition
		}
	}
	
	template<typename HF, typename EF>
	void setCapacity(size_t newCapacity, const HF &hashFunction, const EF &equalityFunction)
	{
		// Store temporarily the data.
		auto oldKeys = keys;
		auto oldValues = values;
		size_t oldCapacity = capacityObject.decodeSmallInteger();
		
		// Create the new capacity.
		capacityObject = Oop::encodeSmallInteger(newCapacity);
		tallyObject = Oop::encodeSmallInteger(0);
		setKeyCapacity(newCapacity);
		setValueCapacity(newCapacity);
		
		// Add back the old objects.
		if(oldKeys != (Array*)&NilObject)
		{
			auto nil = nilOop(); 
			Oop *oldKeysOops = reinterpret_cast<Oop *> (oldKeys->getFirstFieldPointer());
			Oop *oldValuesOops = reinterpret_cast<Oop *> (oldValues->getFirstFieldPointer());
			for(size_t i = 0; i < oldCapacity; ++i)
			{
				auto oldKey = oldKeysOops[i];
				if(oldKey != nil)
					internalAtPut(oldKey, oldValuesOops[i], hashFunction, equalityFunction);
			}
		}
	}
	
	void setValueCapacity(size_t valueCapacity)
	{
		values = Array::basicNativeNew(valueCapacity);
	}

	Array* values;
};

/**
 * MethodDictionary
 */
class MethodDictionary: public Dictionary
{
	LODTALK_NATIVE_CLASS();
public:
	struct End {};

	template<typename ...Args>
	MethodDictionary(Args ... args)
	{
		object_header_ = ObjectHeader::specialNativeClass(generateIdentityHash(this), SCI_MethodDictionary, 4);
		addMethods(args...);
	}
	
	void addMethods(End null)
	{
	}
	
	template<typename T, typename ... Args>
	void addMethods(const T &method, Args... args)
	{
		addMethod(method);
		addMethods(args...);
	}
	
	template<typename T>
	void addMethod(const T &methodDescriptor)
	{
		atPut(methodDescriptor.getSelector(), methodDescriptor.getMethod());
	}

	Oop atOrNil(Oop key)
	{
		return internalAtOrNil(key, identityHashOf, identityOopEquals);
	}
		
	Oop atPut(Oop key, Oop value)
	{
		internalAtPut(key, value, identityHashOf, identityOopEquals);
		return value;
	}
};

/**
 * IdentityDictionary
 */
class IdentityDictionary: public Dictionary
{
	LODTALK_NATIVE_CLASS();
public:
	struct End {};

	IdentityDictionary()
	{
		object_header_ = ObjectHeader::specialNativeClass(generateIdentityHash(this), SCI_IdentityDictionary, 4);
	}

	Oop atOrNil(Oop key)
	{
		return internalAtOrNil(key, identityHashOf, identityOopEquals);
	}
		
	Oop atPut(Oop key, Oop value)
	{
		internalAtPut(key, value, identityHashOf, identityOopEquals);
		return value;
	}
};

/**
 * SystemDictionary
 */
class SystemDictionary: public IdentityDictionary
{
	LODTALK_NATIVE_CLASS();
public:
	struct End {};

	SystemDictionary()
	{
		object_header_ = ObjectHeader::specialNativeClass(generateIdentityHash(this), SCI_SystemDictionary, 4);
	}

	Oop atOrNil(Oop key)
	{
		return internalAtOrNil(key, identityHashOf, identityOopEquals);
	}
		
	Oop atPut(Oop key, Oop value)
	{
		internalAtPut(key, value, identityHashOf, identityOopEquals);
		return value;
	}
};

} // End of namespace Lodtalk

#endif //LODTALK_COLLECTIONS_HPP_
