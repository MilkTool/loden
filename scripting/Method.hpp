#ifndef LODTALK_METHOD_HPP_
#define LODTALK_METHOD_HPP_

#include <type_traits>
#include <vector>
#include "Object.hpp"
#include "Collections.hpp"
#include "MPL.hpp"

namespace Lodtalk
{
class NativeMethodWrapper;

/**
 * Compiled method
 */
class CompiledMethod: public ByteArray
{
	LODTALK_NATIVE_CLASS();	
public:
	static CompiledMethod *newMethodWithHeader(size_t numberOfBytes, CompiledMethodHeader header);

	Oop execute(Oop receiver, int argumentCount, Oop *arguments);
	
	CompiledMethodHeader *getHeader()
	{
		return reinterpret_cast<CompiledMethodHeader*> (getFirstFieldPointer());
	}
	
	size_t getLiteralCount()
	{
		return getHeader()->getLiteralCount();
	}
	
	size_t getTemporalCount()
	{
		return getHeader()->getTemporalCount();
	}
	
	size_t getArgumentCount()
	{
		return getHeader()->getArgumentCount();
	}
	
	size_t getFirstPC()
	{
		return (getHeader()->getLiteralCount() + 1)*sizeof(void*);
	}
	
	uint8_t *getFirstBCPointer()
	{
		return reinterpret_cast<uint8_t*> (getFirstFieldPointer() + getFirstPC());
	}
};

/**
 * Native method
 */
class NativeMethod: public Object
{
	LODTALK_NATIVE_CLASS();
public:
	NativeMethod(NativeMethodWrapper *wrapper)
	{
		object_header_ = ObjectHeader::specialNativeClass(generateIdentityHash(this), SCI_NativeMethod, 1, OF_INDEXABLE_8);
		this->wrapper = wrapper;
	}

	Oop execute(Oop receiver, int argumentCount, Oop *arguments);
	
	NativeMethodWrapper *wrapper;
};

/**
 * Native method wrapper
 */
class NativeMethodWrapper
{
public:
	NativeMethodWrapper() {}
	virtual ~NativeMethodWrapper() {}
	
	virtual Oop execute(Oop receiver, int argumentCount, Oop *arguments) = 0;
};

namespace detail
{
	// Argument marshallers. 
	template<typename T>
	struct NativeArgumentMarshaller;
	
	template<>
	struct NativeArgumentMarshaller<Oop>
	{
		static Oop apply(Oop argument)
		{
			return argument;
		}
	};

	template<typename T>
	struct ExplicitTypeArgumentMarshaller
	{
		static T* apply(Oop argument)
		{
			return reinterpret_cast<T*> (argument.pointer);
		}
	};

	template<typename T>
	struct NativeArgumentMarshaller<T*> :
		MPL::if_<std::is_base_of<ProtoObject, T>::value, ExplicitTypeArgumentMarshaller<T>, MPL::error_base>::type
	{};
	
	// Result marshallers.
	template<typename T>
	struct NativeResultMarshaller;
	
	template<>
	struct NativeResultMarshaller<Oop>
	{
		static Oop apply(Oop result)
		{
			return result;
		}
	};
};

template<typename T>
Oop nativeResultMarshaller(const T &result)
{
	return detail::NativeResultMarshaller<T>::apply(result);
}

template<typename T>
T nativeArgumentMarshaller(Oop object)
{
	return detail::NativeArgumentMarshaller<T>::apply(object);
}

/**
 * Global native method wrapper
 */
template<typename R, typename...Args>
class GlobalNativeMethodWrapper: public NativeMethodWrapper 
{
public:
	typedef R (*FunctionType) (Args...);
	typedef MPL::vector<Args...> FunctionArgumentTypes;
	
	GlobalNativeMethodWrapper(const FunctionType &function)
		: function(function)
	{
	}
	
	virtual Oop execute(Oop receiver, int argumentCount, Oop *arguments) override
	{
		assert(argumentCount + 1== MPL::size<FunctionArgumentTypes>::value);
		return nativeResultMarshaller<R> (addReceiver(receiver, arguments));
	}
	
private:
	R addReceiver(Oop receiver, Oop *arguments)
	{
		typedef typename MPL::front<FunctionArgumentTypes>::type ArgumentType;
		typedef typename MPL::pop_front<FunctionArgumentTypes>::type NextTypes;
		
		return addArguments<NextTypes> (arguments, nativeArgumentMarshaller<ArgumentType> (receiver));
	}
	
	template<typename RemainingTypes, typename...CurrentArgs>
	typename std::enable_if<MPL::size<RemainingTypes>::value != 0, R>::type
	addArguments(Oop *arguments, CurrentArgs... marshalledArguments)
	{
		typedef typename MPL::front<RemainingTypes>::type ArgumentType;
		typedef MPL::vector<CurrentArgs...> CurrentTypes;
		typedef typename MPL::pop_front<RemainingTypes>::type NextTypes;

		auto marshalled = nativeArgumentMarshaller<ArgumentType> (arguments[MPL::size<CurrentTypes>::value - 1]);
		return addArguments<NextTypes> (arguments, marshalledArguments..., marshalled);
	}

	template<typename RemainingTypes, typename...CurrentArgs>
	typename std::enable_if<MPL::size<RemainingTypes>::value == 0, R>::type
	addArguments(Oop *arguments, CurrentArgs... marshalledArguments)
	{
		return function(marshalledArguments...);
	}
	
	FunctionType function;
};

/**
 * Member native method wrapper
 */
template<typename R, typename CT, typename...Args>
class MemberNativeMethodWrapper: public NativeMethodWrapper
{
public:
	typedef R (CT::*FunctionType) (Args...);
	typedef MPL::vector<Args...> FunctionArgumentTypes;

	MemberNativeMethodWrapper(const FunctionType &function)
		: function(function)
	{
	}
	
	virtual Oop execute(Oop receiver, int argumentCount, Oop *arguments) override
	{
		assert(argumentCount == MPL::size<FunctionArgumentTypes>::value);
		return nativeResultMarshaller<R> (addArguments<FunctionArgumentTypes> (receiver, arguments));
	}
	
private:
	template<typename RemainingTypes, typename...CurrentArgs>
	typename std::enable_if<MPL::size<RemainingTypes>::value != 0, R>::type
	addArguments(Oop receiver, Oop *arguments, CurrentArgs... marshalledArguments)
	{
		typedef typename MPL::front<RemainingTypes>::type ArgumentType;
		typedef MPL::vector<CurrentArgs...> CurrentTypes;
		typedef typename MPL::pop_front<RemainingTypes>::type NextTypes;

		auto marshalled = nativeArgumentMarshaller<ArgumentType> (arguments[MPL::size<CurrentTypes>::value]);
		return addArguments<NextTypes> (receiver, arguments, marshalledArguments..., marshalled);
	}

	template<typename RemainingTypes, typename...CurrentArgs>
	typename std::enable_if<MPL::size<RemainingTypes>::value == 0, R>::type
	addArguments(Oop receiver, Oop *arguments, CurrentArgs... marshalledArguments)
	{
		auto self = nativeArgumentMarshaller<CT*> (receiver);
		return (self->*function) (marshalledArguments...);
	}
	
	FunctionType function;
};

namespace detail
{
	template<typename T>
	struct NativeMethodWrapperCreator;
	
	template<typename R, typename... Args>
	struct NativeMethodWrapperCreator<R (*) (Args...)>
	{
		typedef R (*FT) (Args...);
		
		static NativeMethodWrapper *apply(const FT &functionPointer)
		{
			return new GlobalNativeMethodWrapper<R, Args...> (functionPointer);
		}
	};
	
	template<typename R, typename... Args>
	struct NativeMethodWrapperCreator<R (Args...)>
	{
		typedef R (*FT) (Args...);
		
		static NativeMethodWrapper *apply(const FT &functionPointer)
		{
			return new GlobalNativeMethodWrapper<R, Args...> (functionPointer);
		}
	};
	
	template<typename R, typename CT, typename... Args>
	struct NativeMethodWrapperCreator<R (CT::*) (Args...)>
	{
		typedef R (CT::*FT) (Args...);
		
		static NativeMethodWrapper *apply(const FT &functionPointer)
		{
			return new MemberNativeMethodWrapper<R, CT, Args...> (functionPointer);
		}
	};
}

template<typename FT>
NativeMethodWrapper *makeNativeMethodWrapper(const FT &function)
{
	return detail::NativeMethodWrapperCreator<FT>::apply(function);
}


/**
 * Native method descriptor
 */
class NativeMethodDescriptor
{
public:
	NativeMethodDescriptor(const std::string &selectorString, NativeMethodWrapper *methodWrapper)
		: selectorString(selectorString), methodWrapper(methodWrapper)
	{
	}
	
	~NativeMethodDescriptor()
	{
	}
	
	
	Oop getSelector() const;
	Oop getMethod() const;
	
private:
	std::string selectorString;
	NativeMethodWrapper *methodWrapper;
};

template<typename FT>
NativeMethodDescriptor makeNativeMethodDescriptor(const std::string &selector, const FT &method)
{
	return NativeMethodDescriptor(selector, makeNativeMethodWrapper(method));
}

} // End of namespace Lodtalk

#endif //LODTALK_METHOD_HPP_
