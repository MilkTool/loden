#ifndef LODTALK_OBJECT_HPP
#define LODTALK_OBJECT_HPP

#include "ObjectModel.hpp"

namespace Lodtalk
{

#define LODTALK_NATIVE_CLASS() \
public: \
	static ClassDescription * ClassObject; \
	static ClassDescription * MetaclassObject; \


class ClassDescription;
class MethodDictionary;
class Class;
class Metaclass;

/**
 * ProtoObject
 */
class ProtoObject
{
	LODTALK_NATIVE_CLASS();
public:
	ObjectHeader object_header_;
	
	Oop selfOop()
	{
		return Oop::fromPointer(this);
	}
	
	Oop nativePerformWithArguments(Oop selector, int argumentCount, Oop *arguments)
	{
		return sendMessage(selfOop(), selector, argumentCount, arguments);
	}
	
	uint8_t *getFirstFieldPointer()
	{
		uint8_t *result = reinterpret_cast<uint8_t *> (&object_header_);
		result += sizeof(ObjectHeader);
		if(object_header_.slotCount == 255)
			result += 8;
		return result;
	}
		
	int identityHashValue() const
	{
		return object_header_.identityHash;
	}
};

/**
 * Object
 */
class Object: public ProtoObject
{
	LODTALK_NATIVE_CLASS();
public:
};

/**
 * Behavior
 */
class Behavior: public Object
{
	LODTALK_NATIVE_CLASS();
public:	
	Behavior* superclass;
	MethodDictionary *methodDict;
	Oop format;
	Oop fixedVariableCount;
	Oop layout;

	Oop basicNew();
	Oop basicNew(Oop indexableSize);

	Object *basicNativeNew();
	Object *basicNativeNew(size_t indexableSize);

	Oop superLookupSelector(Oop selector);
	Oop lookupSelector(Oop selector);
			
protected:
	Behavior(Behavior* superclass, MethodDictionary *methodDict, ObjectFormat format, int fixedVariableCount)
		: superclass(superclass), methodDict(methodDict), format(Oop::encodeSmallInteger((int)format)), fixedVariableCount(Oop::encodeSmallInteger(fixedVariableCount))
	{
	}
};

/**
 * ClassDescription
 */
class ClassDescription: public Behavior
{
	LODTALK_NATIVE_CLASS();	
protected:
	ClassDescription(Behavior* superclass, MethodDictionary *methodDict, ObjectFormat format, int fixedVariableCount)
		: Behavior(superclass, methodDict, format, fixedVariableCount)
	{
	}
};

/**
 * Class
 */
class Class: public ClassDescription
{
	LODTALK_NATIVE_CLASS();
public:
	Class(const char *className, unsigned int classId, unsigned int metaclassId, ClassDescription *metaClass, Behavior* superclass, MethodDictionary *methodDict, ObjectFormat format, int fixedVariableCount)
		: ClassDescription(superclass, methodDict, format, fixedVariableCount)
	{
		object_header_ = ObjectHeader::specialNativeClass(classId, metaclassId, 5);
		setGlobalVariable(className, Oop::fromPointer(this));
	}
};

/**
 * Metaclass
 */
class Metaclass: public ClassDescription
{
	LODTALK_NATIVE_CLASS();
public:
	Metaclass(unsigned int classId, Behavior* superclass, MethodDictionary *methodDict, int fixedVariableCount)
		: ClassDescription(superclass, methodDict, OF_FIXED_SIZE, 5 + fixedVariableCount)
	{
		object_header_ = ObjectHeader::specialNativeClass(classId, SCI_Metaclass, 5);
	}
};

class MethodDictionary;

/**
 * UndefinedObject
 */
class UndefinedObject: public Object
{
	LODTALK_NATIVE_CLASS();
public:
	UndefinedObject()
	{
		object_header_ = ObjectHeader::emptySpecialNativeClass(SOI_Nil, SCI_UndefinedObject);
	}
};

/**
 * Boolean
 */
class Boolean: public Object
{
	LODTALK_NATIVE_CLASS();
};

/**
 * True
 */
class True: public Boolean
{
	LODTALK_NATIVE_CLASS();
public:
	True()
	{
		object_header_ = ObjectHeader::emptySpecialNativeClass(SOI_True, SCI_True);
	}
};

/**
 * False
 */
class False: public Boolean
{
	LODTALK_NATIVE_CLASS();
public:
	False()
	{
		object_header_ = ObjectHeader::emptySpecialNativeClass(SOI_False, SCI_False);
	}
};

/**
 * Magnitude
 */
class Magnitude: public Object
{
	LODTALK_NATIVE_CLASS();
};

/**
 * Number
 */
class Number: public Magnitude
{
	LODTALK_NATIVE_CLASS();
};

/**
 * Integer
 */
class Integer: public Number
{
	LODTALK_NATIVE_CLASS();
};

/**
 * SmallInteger
 */
class SmallInteger: public Integer
{
	LODTALK_NATIVE_CLASS();
};

/**
 * Float
 */
class Float: public Number
{
	LODTALK_NATIVE_CLASS();
};

/**
 * SmallFloat
 */
class SmallFloat: public Float
{
	LODTALK_NATIVE_CLASS();
};

/**
 * Character
 */
class Character: public Magnitude
{
	LODTALK_NATIVE_CLASS();
};

/**
 * Lookup key
 */
class LookupKey: public Magnitude
{
	LODTALK_NATIVE_CLASS();
public:
	Oop key;
};

inline Oop getLookupKeyKey(const Oop lookupKey)
{
	return reinterpret_cast<LookupKey*> (lookupKey.pointer)->key; 
}

/**
 * Association
 */
class Association: public LookupKey
{
	LODTALK_NATIVE_CLASS();
	Association() {}
public:
	static Association *newNativeKeyValue(int clazzId, Oop key, Oop value);

	Oop value;
};

/**
 * LiteralVariable
 */
class LiteralVariable: public Association
{
	LODTALK_NATIVE_CLASS();
public:
};

/**
 * GlobalVariable
 */
class GlobalVariable: public LiteralVariable
{
	LODTALK_NATIVE_CLASS();
public:
	static Association* make(Oop key, Oop value)
	{
		return newNativeKeyValue(SCI_GlobalVariable, key, value);
	}
};

/**
 * ClassVariable
 */
class ClassVariable: public LiteralVariable
{
	LODTALK_NATIVE_CLASS();
public:
	static Association* make(Oop key, Oop value)
	{
		return newNativeKeyValue(SCI_ClassVariable, key, value);
	}

};

// Class method dictionary.
#define LODTALK_BEGIN_CLASS_TABLE(className) \
static MethodDictionary className ## _class_methodDict = MethodDictionary(

#define LODTALK_METHOD(selector, methodImplementation) \
	makeNativeMethodDescriptor(selector, methodImplementation),
 
#define LODTALK_END_CLASS_TABLE() MethodDictionary::End() );

// Metaclass method dictionary.
#define LODTALK_BEGIN_CLASS_SIDE_TABLE(className) \
static MethodDictionary className ## _metaclass_methodDict = MethodDictionary( \

#define LODTALK_END_CLASS_SIDE_TABLE() MethodDictionary::End() );

// Special class and meta class definition
#define LODTALK_SPECIAL_METACLASS_DEFINITION(className, superName, fixedVariableCount) \
static Metaclass className ## _metaclass (SMCI_ ##className, superName::MetaclassObject, &className ## _metaclass_methodDict, fixedVariableCount); \
ClassDescription *className::MetaclassObject = &className ## _metaclass;

#define LODTALK_SPECIAL_CLASS_DEFINITION(className, superName, format, fixedVariableCount) \
static Class className ## _class (#className, SCI_ ##className, SMCI_ ##className, className::MetaclassObject, superName::ClassObject, &className ## _class_methodDict, format, fixedVariableCount); \
ClassDescription *className::ClassObject = &className ## _class;

#define LODTALK_SPECIAL_SUBCLASS_DEFINITION(className, superName, format, fixedVariableCount) \
LODTALK_SPECIAL_METACLASS_DEFINITION(className, superName, 0) \
LODTALK_SPECIAL_CLASS_DEFINITION(className, superName, format, fixedVariableCount)

// Native method.
#define LODTALK_NATIVE_METHOD(selector, cppImplementation)

// The nil oop

inline bool isNil(ProtoObject *obj)
{
	return (UndefinedObject*)obj == &NilObject;
}

inline bool isNil(Oop obj)
{
	return obj.isNil();
}

} // End of namespace Lodtalk

#endif //LODTALK_OBJECT_HPP
