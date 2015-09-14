#ifndef LODEN_STACK_MEMORY_HPP
#define LODEN_STACK_MEMORY_HPP

#include <vector>
#include <functional>
#include "Object.hpp"

namespace Lodtalk
{
const size_t StackMemoryPageSize = 4096; // 4 KB

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
	
	inline void push(Oop oop)
	{
		stackPointer -= sizeof(Oop);
		*reinterpret_cast<Oop*> (stackPointer) = oop;
	}
	
	inline Oop pop()
	{
		auto res = stackTop();
		stackPointer += sizeof(Oop);
		return res;
	}
	
	inline Oop stackTop()
	{
		return *reinterpret_cast<Oop*> (stackPointer);
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