#ifndef LODEN_OBJECT_HPP
#define LODEN_OBJECT_HPP

#include "Loden/Common.hpp"

namespace Loden
{

LODEN_DECLARE_CLASS(Object);
LODEN_DECLARE_CLASS(ObjectClassFactory);

template<typename T, typename U>
std::shared_ptr<T> querySharedInterface(const std::shared_ptr<U> &ptr)
{
    auto queried = ptr->template queryInterface<T> ();
    if (queried)
        return std::shared_ptr<T>(ptr, queried);
    return nullptr;
}

/**
 * Loden object
 */
class LODEN_CORE_EXPORT Object : public std::enable_shared_from_this<Object>
{
public:
    static const ObjectClassFactory * const ClassFactory;
    static constexpr bool IsAbstract = false;
    static constexpr bool IsInterface = false;

public:
    virtual ~Object();

    virtual const ObjectClassFactory *getClass() const;
    virtual void *queryInterfacePointer(const ObjectClassFactory *clazz);
    virtual const void *getCanonicalPointer() const;

    template<typename T>
    T *queryInterface()
    {
        auto pointer = queryInterfacePointer(&T::ClassFactory);
        return reinterpret_cast<T*> (pointer);
    }
};

/**
 * Object class
 */
class LODEN_CORE_EXPORT ObjectClassFactory
{
public:
    ObjectClassFactory();
    virtual ~ObjectClassFactory();

    virtual const char *getName() const = 0;

    virtual bool isAbstract() const = 0;
    virtual bool isInterface() const = 0;

    virtual Object *construct() = 0;
    virtual ObjectPtr constructShared() = 0;
};

/**
 * Object interface class
 */
template<typename T>
class LODEN_CORE_EXPORT ObjectInterfaceClassFactory : public ObjectClassFactory
{
public:
    ObjectInterfaceClassFactory(const char *name)
        : name(name)
    {
    }

    ~ObjectInterfaceClassFactory()
    {
    }

    const char *getName() const
    {
        return name;
    }

    virtual bool isAbstract() const
    {
        return true;
    }

    virtual bool isInterface() const
    {
        return true;
    }

    virtual Object *construct() override
    {
        // Should never reach here.
        abort();
    }

    virtual ObjectPtr constructShared() override
    {
        // Should never reach here.
        abort();
    }

private:
    const char *name;
};

/**
 * Object abstract class factory
 */
template<typename T>
class LODEN_CORE_EXPORT ObjectAbstractClassFactory : public ObjectClassFactory
{
public:
    ObjectAbstractClassFactory(const char *name)
        : name(name)
    {
    }

    ~ObjectAbstractClassFactory()
    {
    }

    const char *getName() const
    {
        return name;
    }

    virtual bool isAbstract() const
    {
        return true;
    }

    virtual bool isInterface() const
    {
        return false;
    }

    virtual Object *construct() override
    {
        // Should never reach here.
        abort();
    }

    virtual ObjectPtr constructShared() override
    {
        // Should never reach here.
        abort();
    }

private:
    const char *name;
};

/**
 * Object native class
 */
template<typename T>
class ObjectNativeClassFactory : public ObjectClassFactory
{
public:
    ObjectNativeClassFactory(const char *name)
        : name(name)
    {
    }

    ~ObjectNativeClassFactory()
    {
    }

    const char *getName() const
    {
        return name;
    }

    virtual bool isAbstract() const
    {
        return false;
    }

    virtual bool isInterface() const
    {
        return false;
    }

    virtual Object *construct() override
    {
        return new T();
    }

    virtual ObjectPtr constructShared() override
    {
        return std::make_shared<T> ();
    }

private:
    const char *name;
};

namespace detail
{

/**
 * Implement interfaces
 */

template<typename... Args>
class ObjectImplementInterfaces;

template<typename ST>
class ObjectImplementInterfaces<ST>
{
public:
    virtual void *queryInterfacePointer(const ObjectClassFactory *clazz)
    {
        return nullptr;
    }
};

template<typename ST, typename Interface, typename... Rest>
class ObjectImplementInterfaces<ST, Interface, Rest...>: public virtual Interface, public ObjectImplementInterfaces<ST, Rest...>
{
private:
    typedef ObjectImplementInterfaces<ST, Rest...> RestImplement;

public:
    virtual void *queryInterfacePointer(const ObjectClassFactory *clazz)
    {
        auto result = Interface::queryInterfacePointer(clazz);
        if(result)
            return result;
        return RestImplement::queryInterfacePointer(clazz);
    }
};

template<typename... Args>
struct FirstTypeOf;

template<typename First, typename... Rest>
struct FirstTypeOf<First, Rest...>
{
    typedef First type;
};

} // end of namespace detail

template<typename ST, typename... Interfaces>
class ObjectSubclass: public detail::ObjectImplementInterfaces<ST, Interfaces...>
{
private:
    typedef detail::ObjectImplementInterfaces<ST, Interfaces...> InterfaceImplement;

    static ObjectNativeClassFactory<ST> ClassFactoryImplementation;

public:
    static const ObjectClassFactory * const ClassFactory;
    static constexpr bool IsAbstract = false;
    static constexpr bool IsInterface = false;

    typedef ST SelfType;
    typedef typename detail::FirstTypeOf<Interfaces...>::type BaseType;

public:
    virtual const void *getCanonicalPointer() const override
    {
        return this;
    }

    virtual void *queryInterfacePointer(const ObjectClassFactory *clazz) override
    {
        if(clazz == ST::ClassFactory)
            return reinterpret_cast<void*> (static_cast<ST*> (this));
        return InterfaceImplement::queryInterfacePointer(clazz);
    }

    std::shared_ptr<ST> sharedFromThis()
    {
        return std::shared_ptr<ST>(Object::shared_from_this(), static_cast<ST*> (this));
    }

    std::shared_ptr<const ST> sharedFromThis() const
    {
        return std::shared_ptr<const ST>(Object::shared_from_this(), static_cast<const ST*> (this));
    }

};

template<typename ST, typename... Interfaces>
ObjectNativeClassFactory<ST> ObjectSubclass<ST, Interfaces...>::ClassFactoryImplementation(ST::ClassName);

template<typename ST, typename... Interfaces>
const ObjectClassFactory * const ObjectSubclass<ST, Interfaces...>::ClassFactory = &ClassFactoryImplementation;

template<typename ST, typename... Interfaces>
class ObjectAbstractSubclass: public detail::ObjectImplementInterfaces<ST, Interfaces...>
{
private:
    typedef detail::ObjectImplementInterfaces<ST, Interfaces...> InterfaceImplement;

    static ObjectAbstractClassFactory<ST> ClassFactoryImplementation;

public:
    static const ObjectClassFactory * const ClassFactory;
    static constexpr bool IsAbstract = true;
    static constexpr bool IsInterface = false;

    typedef ST SelfType;
    typedef typename detail::FirstTypeOf<Interfaces...>::type BaseType;

public:
    virtual void *queryInterfacePointer(const ObjectClassFactory *clazz) override
    {
        if(clazz == ST::ClassFactory)
            return reinterpret_cast<void*> (static_cast<ST*> (this));
        return InterfaceImplement::queryInterfacePointer(clazz);
    }
};

template<typename ST, typename... Interfaces>
ObjectAbstractClassFactory<ST> ObjectAbstractSubclass<ST, Interfaces...>::ClassFactoryImplementation(ST::ClassName);

template<typename ST, typename... Interfaces>
const ObjectClassFactory * const ObjectAbstractSubclass<ST, Interfaces...>::ClassFactory = &ClassFactoryImplementation;

template<typename ST, typename... Interfaces>
class ObjectInterfaceSubclass: public detail::ObjectImplementInterfaces<ST, Interfaces...>
{
private:
    typedef detail::ObjectImplementInterfaces<ST, Interfaces...> InterfaceImplement;

    static ObjectInterfaceClassFactory<ST> ClassFactoryImplementation;

public:
    static const ObjectClassFactory * const ClassFactory;
    static constexpr bool IsAbstract = true;
    static constexpr bool IsInterface = true;

public:
    virtual void *queryInterfacePointer(const ObjectClassFactory *clazz) override
    {
        if(clazz == ST::ClassFactory)
            return reinterpret_cast<void*> (static_cast<ST*> (this));
        return InterfaceImplement::queryInterfacePointer(clazz);
    }
};

template<typename ST, typename... Interfaces>
ObjectInterfaceClassFactory<ST> ObjectInterfaceSubclass<ST, Interfaces...>::ClassFactoryImplementation(ST::ClassName);

template<typename ST, typename... Interfaces>
const ObjectClassFactory * const ObjectInterfaceSubclass<ST, Interfaces...>::ClassFactory = &ClassFactoryImplementation;

} // End of namespace Loden

#endif //LODEN_OBJECT_HPP
