#include <string.h>
#include "Object.hpp"
#include "Collections.hpp"
#include "Method.hpp"

namespace Lodtalk
{

// Special object
UndefinedObject NilObject;
True TrueObject;
False FalseObject;

// Proto object methods
LODTALK_BEGIN_CLASS_SIDE_TABLE(ProtoObject)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(ProtoObject)
LODTALK_END_CLASS_TABLE()

// Proto object method dictionary.
static Metaclass ProtoObject_metaclass(SMCI_ProtoObject, Class::ClassObject, &ProtoObject_class_methodDict, 0);
ClassDescription *ProtoObject::MetaclassObject = &ProtoObject_metaclass;

static Class ProtoObject_class("ProtoObject", SCI_ProtoObject, SMCI_ProtoObject, &ProtoObject_metaclass, (Behavior*)&NilObject, &ProtoObject_class_methodDict, OF_EMPTY, 0);
ClassDescription *ProtoObject::ClassObject = &ProtoObject_class;

// Object methods
LODTALK_BEGIN_CLASS_SIDE_TABLE(Object)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(Object)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(Object, ProtoObject, OF_EMPTY, 0);

// Undefined object
LODTALK_BEGIN_CLASS_SIDE_TABLE(UndefinedObject)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(UndefinedObject)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(UndefinedObject, Object, OF_EMPTY, 0);

// Behavior
Oop Behavior::basicNew()
{
	return Oop::fromPointer(basicNativeNew());
}

Oop Behavior::basicNew(Oop indexableSize)
{
	return Oop::fromPointer(basicNativeNew(indexableSize.decodeSmallInteger()));
}

Object *Behavior::basicNativeNew(size_t indexableSize)
{
	auto theFormat = (ObjectFormat)format.decodeSmallInteger();
	auto fixedSlotCount = fixedVariableCount.decodeSmallInteger();
	auto classIndex = object_header_.identityHash;
	return reinterpret_cast<Object*> (newObject(fixedSlotCount, indexableSize, theFormat, classIndex));
}

Object *Behavior::basicNativeNew()
{
	return basicNativeNew(0);
}

Oop Behavior::superLookupSelector(Oop selector)
{
	// Find in the super class.
	if(isNil(superclass))
		return nilOop();
	return superclass->lookupSelector(selector);
}

Oop Behavior::lookupSelector(Oop selector)
{
	// Sanity check.
	if(isNil(methodDict))
		return nilOop();

	// Look the method in the dictionary.
	auto method = methodDict->atOrNil(selector);
	if(!isNil(method))
		return method;

	// Find in the super class.
	if(isNil(superclass))
		return nilOop();
	return superclass->lookupSelector(selector);
}

LODTALK_BEGIN_CLASS_SIDE_TABLE(Behavior)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(Behavior)
	LODTALK_METHOD("basicNew", static_cast<Oop (Behavior::*)()> (&Behavior::basicNew))
	LODTALK_METHOD("basicNew:", static_cast<Oop (Behavior::*)(Oop)> (&Behavior::basicNew))
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(Behavior, Object, OF_FIXED_SIZE, 5);

// ClassDescription 
LODTALK_BEGIN_CLASS_SIDE_TABLE(ClassDescription)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(ClassDescription)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(ClassDescription, Behavior, OF_FIXED_SIZE, 5);

// Class
LODTALK_BEGIN_CLASS_SIDE_TABLE(Class)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(Class)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(Class, ClassDescription, OF_FIXED_SIZE, 5);

// Metaclass
LODTALK_BEGIN_CLASS_SIDE_TABLE(Metaclass)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(Metaclass)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(Metaclass, ClassDescription, OF_FIXED_SIZE, 5);

// Boolean
LODTALK_BEGIN_CLASS_SIDE_TABLE(Boolean)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(Boolean)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(Boolean, Object, OF_EMPTY, 0);

// True
LODTALK_BEGIN_CLASS_SIDE_TABLE(True)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(True)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(True, Boolean, OF_EMPTY, 0);

// False
LODTALK_BEGIN_CLASS_SIDE_TABLE(False)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(False)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(False, Boolean, OF_EMPTY, 0);

// Magnitude
LODTALK_BEGIN_CLASS_SIDE_TABLE(Magnitude)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(Magnitude)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(Magnitude, Object, OF_EMPTY, 0);

// Number
LODTALK_BEGIN_CLASS_SIDE_TABLE(Number)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(Number)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(Number, Magnitude, OF_EMPTY, 0);

// Integer
LODTALK_BEGIN_CLASS_SIDE_TABLE(Integer)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(Integer)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(Integer, Number, OF_EMPTY, 0);

// SmallInteger
LODTALK_BEGIN_CLASS_SIDE_TABLE(SmallInteger)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(SmallInteger)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(SmallInteger, Integer, OF_EMPTY, 0);

// Float
LODTALK_BEGIN_CLASS_SIDE_TABLE(Float)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(Float)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(Float, Number, OF_EMPTY, 0);

// SmallFloat
LODTALK_BEGIN_CLASS_SIDE_TABLE(SmallFloat)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(SmallFloat)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(SmallFloat, Float, OF_EMPTY, 0);

// Character
LODTALK_BEGIN_CLASS_SIDE_TABLE(Character)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(Character)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(Character, Magnitude, OF_EMPTY, 0);

// LookupKey
LODTALK_BEGIN_CLASS_SIDE_TABLE(LookupKey)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(LookupKey)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(LookupKey, Magnitude, OF_FIXED_SIZE, 1);

// Association
Association* Association::newNativeKeyValue(int classIndex, Oop key, Oop value)
{
	auto assoc = reinterpret_cast<Association*> (newObject(2, 0, OF_FIXED_SIZE, classIndex));
	assoc->key = key;
	assoc->value = value;
	return assoc;
}

LODTALK_BEGIN_CLASS_SIDE_TABLE(Association)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(Association)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(Association, LookupKey, OF_FIXED_SIZE, 2);

// LiteralVariable
LODTALK_BEGIN_CLASS_SIDE_TABLE(LiteralVariable)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(LiteralVariable)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(LiteralVariable, Association, OF_FIXED_SIZE, 2);

// GlobalVariable
LODTALK_BEGIN_CLASS_SIDE_TABLE(GlobalVariable)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(GlobalVariable)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(GlobalVariable, LiteralVariable, OF_FIXED_SIZE, 2);

// ClassVariable
LODTALK_BEGIN_CLASS_SIDE_TABLE(ClassVariable)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(ClassVariable)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(ClassVariable, LiteralVariable, OF_FIXED_SIZE, 2);

} // End of namespace Lodtalk
