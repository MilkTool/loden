#ifndef LODTALK_OBJECT_MODEL_HPP_
#define LODTALK_OBJECT_MODEL_HPP_

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string>

#if UINTPTR_MAX > UINT32_MAX
#define OBJECT_MODEL_SPUR_64 1
#define LODTALK_HAS_SMALLFLOAT 1
#else
#define OBJECT_MODEL_SPUR_32 1
#endif

namespace Lodtalk
{
// Some special constants
const size_t PageSize = 4096;
const size_t PageSizeMask = PageSize - 1;
const size_t PageSizeShift = 12;

// Object constants
struct ObjectTag
{
#ifdef OBJECT_MODEL_SPUR_64
	static const uintptr_t PointerMask = 7;
	static const uintptr_t PointerValue = 0;

	static const uintptr_t SmallInteger = 1;
	static const uintptr_t SmallIntegerMask = 1;
	static const uintptr_t SmallIntegerShift = 1;

	static const uintptr_t Character = 2;
	static const uintptr_t CharacterMask = 7;
	static const uintptr_t CharacterShift = 3;

	static const uintptr_t SmallFloat = 4;
	static const uintptr_t SmallFloatMask = 7;
	static const uintptr_t SmallFloatShift = 3;
#else
	static const uintptr_t PointerMask = 3;
	static const uintptr_t PointerValue = 0;

	static const uintptr_t SmallInteger = 1;
	static const uintptr_t SmallIntegerMask = 1;
	static const uintptr_t SmallIntegerShift = 1;

	static const uintptr_t Character = 2;
	static const uintptr_t CharacterMask = 3;
	static const uintptr_t CharacterShift = 2;
#endif
};

static const uintptr_t IdentityHashMask = (1<<22) - 1;

typedef intptr_t SmallIntegerValue;

struct ObjectHeader;
struct Oop
{
	static Oop fromPointer(void *pointer)
	{
		Oop oop;
		oop.pointer = reinterpret_cast<uint8_t*> (pointer);
		return oop;
	}
	
	inline bool isSmallInteger() const
	{
		return (uintValue & ObjectTag::SmallIntegerMask) == ObjectTag::SmallInteger;
	}
	
	inline bool isCharacter() const
	{
		return (uintValue & ObjectTag::CharacterMask) == ObjectTag::Character;
	}
	
	inline bool isSmallFloat() const
	{
	#ifdef LODTALK_HAS_SMALLFLOAT
		return (uintValue & ObjectTag::SmallFloatMask) == ObjectTag::SmallFloat;
	#else
		return false;
	#endif
	}
	
	inline SmallIntegerValue decodeSmallInteger() const
	{
		assert(isSmallInteger(oop));
		return intValue >> ObjectTag::SmallIntegerShift;
	}
	
	inline static Oop encodeSmallInteger(SmallIntegerValue integer)
	{
		Oop res;
		res.intValue = (integer << ObjectTag::SmallIntegerShift) | ObjectTag::SmallInteger;
		return res;
	}
	
	inline int decodeCharacter() const
	{
		assert(isCharacter());
		return intValue >> ObjectTag::CharacterShift;
	}
	
	inline static Oop encodeCharacter(int character)
	{
		Oop res;
		res.intValue = (character << ObjectTag::CharacterShift) | ObjectTag::Character;
		return res; 
	}

	bool operator==(const Oop &o) const
	{
		return pointer == o.pointer;
	}

	bool operator!=(const Oop &o) const
	{
		return pointer != o.pointer;
	}
	
	union
	{
		uint8_t *pointer;
		ObjectHeader *header;
		uintptr_t uintValue;
		uintptr_t intValue;
	};
};

// Ensure the object oriented pointer is a pointer.
static_assert(sizeof(Oop) == sizeof(void*), "Oop structure has to be a pointer.");

inline unsigned int generateIdentityHash(void *ptr)
{
	return (unsigned int)(reinterpret_cast<uintptr_t> (ptr) & IdentityHashMask);
}

enum ObjectFormat
{
	OF_EMPTY = 0,
	OF_FIXED_SIZE = 1,
	OF_VARIABLE_SIZE_NO_IVARS = 2,
	OF_VARIABLE_SIZE_IVARS = 3,
	OF_WEAK_VARIABLE_SIZE = 4,
	OF_WEAK_FIXED_SIZE = 5,
	OF_INDEXABLE_64 = 9,
	OF_INDEXABLE_32 = 10,
	OF_INDEXABLE_16 = 12,
	OF_INDEXABLE_8 = 16,
	OF_COMPILED_METHOD = 24,
	
	OF_INDEXABLE_NATIVE_FIRST = OF_INDEXABLE_64,
};

inline size_t variableSlotSizeFor(ObjectFormat format)
{
	switch(format)
	{
	case OF_EMPTY:
	case OF_FIXED_SIZE:
		return 0;
	case OF_VARIABLE_SIZE_NO_IVARS:
	case OF_VARIABLE_SIZE_IVARS:
	case OF_WEAK_VARIABLE_SIZE:
		return sizeof(Oop);
	case OF_WEAK_FIXED_SIZE:
		return 0;
	case OF_INDEXABLE_64: return 8;
	case OF_INDEXABLE_32: return 4;
	case OF_INDEXABLE_16: return 2;
	case OF_INDEXABLE_8: return 1;
	case OF_COMPILED_METHOD: return 1;
	default: abort();
	}
};

inline size_t variableSlotDivisor(ObjectFormat format)
{
#ifdef OBJECT_MODEL_SPUR_64
	switch(format)
	{
	case OF_INDEXABLE_64: return 1;
	case OF_INDEXABLE_32: return 2;
	case OF_INDEXABLE_16: return 4;
	case OF_INDEXABLE_8: return 8;
	case OF_COMPILED_METHOD: return 8;
	default: abort();
	}
#else
	switch(format)
	{
	case OF_INDEXABLE_32: return 1;
	case OF_INDEXABLE_16: return 2;
	case OF_INDEXABLE_8: return 4;
	case OF_COMPILED_METHOD: return 8;
	default: abort();
	}
#endif
};

struct ObjectHeader
{
	uint8_t slotCount;
	unsigned int isImmutable : 1;
	unsigned int isPinned : 1;
	unsigned int identityHash : 22;
	unsigned int gcColor : 3;
	unsigned int objectFormat : 5;
	unsigned int reserved : 2;
	unsigned int classIndex : 22;

	static ObjectHeader specialNativeClass(unsigned int identityHash, unsigned int classIndex, uint8_t slotCount, ObjectFormat format = OF_FIXED_SIZE)
	{
		return {slotCount, true, true, identityHash, 0, (unsigned int)format, 0, classIndex};
	}

	static ObjectHeader emptySpecialNativeClass(unsigned int identityHash, unsigned int classIndex)
	{
		return {0, true, true, identityHash, 0, OF_EMPTY, 0, classIndex};
	}
	
	static ObjectHeader emptyNativeClass(void *self, unsigned int classIndex)
	{
		return {0, true, true, generateIdentityHash(self), 0, OF_EMPTY, 0, classIndex};
	}
};

// Object memory
uint8_t *allocateObjectMemory(size_t objectSize);
ObjectHeader *newObject(size_t fixedSlotCount, size_t indexableSize, ObjectFormat format, int classIndex, int identityHash = -1);

/**
 * Object header when the slot count is greater or equal to 255. 
 */
struct BigObjectHeader
{
	ObjectHeader header;
	uint64_t slotCount;
};

enum SpecialObjectIndex
{
	SOI_Nil = 0,
	SOI_True,
	SOI_False,
};

enum SpecialClassesIndex
{
#define SPECIAL_CLASS_NAME(className) \
	SCI_ ## className, \
	SMCI_ ## className,
#include "SpecialClasses.inc"
#undef SPECIAL_CLASS_NAME
};

class ClassDescription;
extern const Oop *specialObjectTable;
extern ClassDescription ** const specialClassTable;

// Gets the identity hash of an Object
inline int identityHashOf(Oop obj)
{
	if(obj.isSmallInteger())
		return obj.decodeSmallInteger();
	if(obj.isCharacter())
		return obj.decodeCharacter();
	//if(isSmallFloat(obj))
	//	return decodeSmallFloat(obj);
	return obj.header->identityHash;
}

inline bool identityOopEquals(Oop a, Oop b)
{
	return a == b;
}

inline int classIndexOf(Oop obj)
{
	if(obj.isSmallInteger())
		return SCI_SmallInteger;
	if(obj.isCharacter())
		return SCI_Character;
	if(obj.isSmallFloat())
		return SCI_SmallFloat;

	return obj.header->classIndex;
}

// Reference smart pointer
template<typename T>
class Ref
{
public:
	Ref()
		: pointer(nullptr) {}
	Ref(decltype(nullptr) ni)
		: pointer(nullptr) {}
	template<typename U>
	Ref(const Ref<U> &o)
		: pointer(o.get()) {}
	explicit Ref(T* p)
		: pointer(p) {}
		
	static Ref<T> fromOop(Oop oop)
	{
		Ref<T> result;
		result.oop = oop;
		return result;
	}

	T *operator->() const
	{
		return pointer;
	}

	T *get() const
	{
		return pointer;
	}
	
	Oop getOop() const
	{
		return oop;
	}

	template<typename U>
	Ref<T> &operator=(const Ref<U> &o)
	{
		pointer = o.get();
		return *this;
	}
	
	bool isSmallInteger() const
	{
		return oop.isSmallInteger();
	}

	bool isSmallFloat() const
	{
		return oop.isSmallFloat();
	}
	
	bool isCharacter() const
	{
		return oop.isCharacter();
	}
	
private:
	union
	{
		T *pointer;
		Oop oop;
	};
};

template<typename T>
Ref<T> makeRef(T *pointer)
{
	return Ref<T> (pointer);
}

// Some object creation / accessing
class ProtoObject;
Ref<ProtoObject> makeIntegerObject(int value);
Ref<ProtoObject> makeFloatObject(double value);
inline Ref<ProtoObject> makeCharacterObject(int value)
{
	return Ref<ProtoObject>::fromOop(Oop::encodeCharacter(value));
}

int64_t readIntegerObject(const Ref<Oop> &ref);
double readDoubleObject(const Ref<Oop> &ref);

// Class table
Oop getClassFromIndex(int classIndex);

// Message send
Oop sendDNUMessage(Oop receiver, Oop selector, int argumentCount, Oop *arguments);
Oop sendMessage(Oop receiver, Oop selector, int argumentCount, Oop *arguments);

Oop makeSelector(const std::string &content);
Oop sendBasicNew(Oop clazz);
Oop sendBasicNewWithSize(Oop clazz, size_t size);


} // End of namespace Lodtalk

#endif //LODTALK_OBJECT_MODEL_HPP_