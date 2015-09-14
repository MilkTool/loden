#ifndef LODEN_STACK_MEMORY_HPP
#define LODEN_STACK_MEMORY_HPP

#include <vector>
#include <functional>
#include "Object.hpp"

namespace Lodtalk
{
static const size_t StackMemoryPageSize = 4096; // 4 KB

class CompiledMethod;

namespace InterpreterStackFrame
{
	static const int WordSize = sizeof(void*);
	
	static const int LastArgumentOffset = 2*WordSize;
	static const int ReturnInstructionPointerOffset = WordSize;
	static const int PrevFramePointerOffset = 0;
	static const int MethodOffset = -WordSize;
	static const int MetadataOffset = -2*WordSize;
	static const int ThisContextOffset = -3*WordSize;
	static const int ReceiverOffset = -4*WordSize;
	static const int FirstTempOffset = -5*WordSize;
}

inline uintptr_t encodeFrameMetaData(bool hasContext, bool isBlock, size_t numArguments)
{
	return (numArguments & 0xFF) |
		  ((isBlock & 0xFF) << 8) |
		  ((hasContext & 0xFF) << 16); 
}

inline void decodeFrameMetaData(uintptr_t metadata, bool &hasContext, bool &isBlock, size_t &numArguments)
{
	numArguments = metadata & 0xFF;
	isBlock = (metadata >> 8) & 0xFF;
	hasContext = (metadata >> 16) & 0xFF;
}


/**
 * Stack memory commited page
 */
class StackMemoryCommitedPage: public Object
{
	LODTALK_NATIVE_CLASS();
public:
};

/**
 * Stack memory for a single thread.
 */
class StackMemory
{
public:
	StackMemory();
	~StackMemory();
	
	void setStorage(uint8_t *storage, size_t storageSize);

public:	
	inline size_t getStackSize() const
	{
		return stackPageHighest - stackPointer;
	}

	inline size_t getAvailableCapacity() const
	{
		return stackPointer - stackPageLowest;
	}
	
	inline void pushOop(Oop oop)
	{
		stackPointer -= sizeof(Oop);
		*reinterpret_cast<Oop*> (stackPointer) = oop;
	}
	
	inline void pushPointer(uint8_t *pointer)
	{
		stackPointer -= sizeof(pointer);
		*reinterpret_cast<uint8_t**> (stackPointer) = pointer;
	}

	inline void pushUInt(uintptr_t value)
	{
		stackPointer -= sizeof(value);
		*reinterpret_cast<uintptr_t*> (stackPointer) = value;
	}

	inline void pushInt(intptr_t value)
	{
		stackPointer -= sizeof(value);
		*reinterpret_cast<intptr_t*> (stackPointer) = value;
	}
	
	inline uint8_t *stackPointerAt(size_t offset)
	{
		return *reinterpret_cast<uint8_t**> (stackPointer + offset);
	}

	inline Oop stackOopAt(size_t offset)
	{
		return *reinterpret_cast<Oop*> (stackPointer + offset);
	}
	
	inline Oop stackTop()
	{
		return *reinterpret_cast<Oop*> (stackPointer);
	}

	inline uintptr_t stackUIntTop(size_t offset)
	{
		return *reinterpret_cast<uintptr_t*> (stackPointer + offset);
	}
	
	inline Oop popOop()
	{
		auto res = stackOopAt(0);
		stackPointer += sizeof(Oop);
		return res;
	}
	
	inline uintptr_t popUInt()
	{
		auto res = stackUIntTop(0);
		stackPointer += sizeof(uintptr_t);
		return res;
	}

	inline void popMultiplesOops(int count)
	{
		stackPointer += count * sizeof(Oop);
	}
	
	inline uint8_t *popPointer()
	{
		auto result = stackPointerAt(0);
		stackPointer += sizeof(uint8_t*);
		return result;
	}
	
	inline uint8_t *getStackPointer()
	{
		return stackPointer;
	}

	inline uint8_t *getFramePointer()
	{
		return framePointer;
	}

	inline void setFramePointer(uint8_t *newPointer)
	{
		framePointer = newPointer;
	}
	
	inline void setStackPointer(uint8_t *newPointer)
	{
		stackPointer = newPointer;
	}
	
	inline uint8_t *getPrevFramePointer()
	{
		return *reinterpret_cast<uint8_t**> (framePointer + InterpreterStackFrame::PrevFramePointerOffset);
	}
	
	inline CompiledMethod *getMethod()
	{
		return *reinterpret_cast<CompiledMethod**> (framePointer + InterpreterStackFrame::MethodOffset);
	}

	inline Oop getReceiver()
	{
		return *reinterpret_cast<Oop*> (framePointer + InterpreterStackFrame::ReceiverOffset);
	}

	inline Oop getThisContext()
	{
		return *reinterpret_cast<Oop*> (framePointer + InterpreterStackFrame::ThisContextOffset);
	}

	inline uintptr_t getMetadata()
	{
		return *reinterpret_cast<uintptr_t*> (framePointer + InterpreterStackFrame::MetadataOffset);
	}

private:
	uint8_t *stackPageLowest;
	uint8_t *stackPageHighest;
	size_t stackPageSize;
	
	uint8_t *framePointer;
	uint8_t *stackPointer;
	
};

typedef std::function<void (StackMemory*) > StackMemoryEntry;

void withStackMemory(const StackMemoryEntry &entryPoint);
std::vector<StackMemory*> getAllStackMemories();
StackMemory *getCurrentStackMemory();

} // End of namespace Lodtalk

#endif //LODEN_STACK_MEMORY_HPP