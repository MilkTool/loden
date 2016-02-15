#include "Loden/Object.hpp"

namespace Loden
{

static ObjectNativeClassFactory<Object> Object_ClassFactory("Object");
const ObjectClassFactory * const Object::ClassFactory = &Object_ClassFactory;

Object::~Object()
{
}

const ObjectClassFactory *Object::getClass() const
{
    return ClassFactory;
}

void *Object::queryInterfacePointer(const ObjectClassFactory *clazz)
{
    if (clazz == &Object_ClassFactory)
        return this;
    return nullptr;
}

const void *Object::getCanonicalPointer() const
{
    return this;
}

ObjectClassFactory::ObjectClassFactory()
{
}

ObjectClassFactory::~ObjectClassFactory()
{
}

} // End of namespace Loden
