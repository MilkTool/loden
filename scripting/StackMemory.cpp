#include <stdlib.h>
#include <vector>
#include <mutex>
#include "Collections.hpp"
#include "Method.hpp"
#include "StackMemory.hpp"

namespace Lodtalk
{
LODTALK_BEGIN_CLASS_SIDE_TABLE(StackMemoryCommitedPage)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(StackMemoryCommitedPage)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(StackMemoryCommitedPage, Object, OF_INDEXABLE_8, 0);

// Stack memory for a single thread.
StackMemory::StackMemory()
{
}

StackMemory::~StackMemory()
{
}

void StackMemory::setStorage(uint8_t *storage, size_t storageSize)
{
	stackPageSize = storageSize;
	stackPageHighest = storage + storageSize;
	stackPageLowest = storage;
	framePointer = nullptr;
	stackPointer = stackPageHighest;	
}

// Stack memories interface used by the GC
class StackMemories
{
public:
	std::vector<StackMemory*> getAll()
	{
		std::unique_lock<std::mutex> l(mutex);
		return memories;
	}

	void registerMemory(StackMemory* memory)
	{
		std::unique_lock<std::mutex> l(mutex);
		memories.push_back(memory);
	}
	
	void unregisterMemory(StackMemory* memory)
	{
		std::unique_lock<std::mutex> l(mutex);
		for(size_t i = 0; i < memories.size(); ++i)
		{
			if(memories[i] == memory)
			{
				memories.erase(memories.begin() + i);
				return;
			}
		}
	}
	
private:
	std::mutex mutex;
	std::vector<StackMemory*> memories;
};

static StackMemories *allStackMemories = nullptr;
static StackMemories *getStackMemoriesData()
{
	if(!allStackMemories)
		allStackMemories = new StackMemories();
	return allStackMemories;
}

// Interface for accessing the stack memory for the current native thread.
static thread_local StackMemory *currentStackMemory = nullptr;

void withStackMemory(const StackMemoryEntry &entryPoint)
{
	if(currentStackMemory)
		return entryPoint(currentStackMemory);
		
	// Ensure the current stack memory pointer is clear when I finish.
	struct EnsureBlock
	{
		~EnsureBlock()
		{
			getStackMemoriesData()->unregisterMemory(currentStackMemory);
			delete currentStackMemory;
			currentStackMemory = nullptr;
		}
	} ensure;
	
	currentStackMemory = new StackMemory();
	currentStackMemory->setStorage(reinterpret_cast<uint8_t*> (alloca(StackMemoryPageSize)), StackMemoryPageSize);
	getStackMemoriesData()->registerMemory(currentStackMemory);
	return entryPoint(currentStackMemory);
}

std::vector<StackMemory*> getAllStackMemories()
{
	return getStackMemoriesData()->getAll();
}

StackMemory *getCurrentStackMemory()
{
	return currentStackMemory;
}

} // End of namespace Lodtalk