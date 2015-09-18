#include <vector>
#include <list>
#include <utility>
#include <mutex>
#include <string.h>
#include "Common.hpp"
#include "ObjectModel.hpp"
#include "Object.hpp"
#include "Collections.hpp"
#include "Method.hpp"

#include "Compiler.hpp"
#include "InputOutput.hpp"
#include "StackMemory.hpp"

namespace Lodtalk
{

static std::vector<Oop> classTable;
static std::vector<Oop> symbolTable;

// Special runtime objects
class SpecialRuntimeObjects
{
public:
	SpecialRuntimeObjects();
	~SpecialRuntimeObjects();

	void createSpecialObjectTable();
	void createSpecialClassTable();

	std::vector<Oop> specialObjectTable;
	std::vector<ClassDescription*> specialClassTable;

    size_t blockActivationSelectorFirst;
    size_t blockActivationSelectorCount;
};

static SpecialRuntimeObjects *theSpecialRuntimeObjects = nullptr;
SpecialRuntimeObjects *getSpecialRuntimeObjects()
{
	if(!theSpecialRuntimeObjects)
		theSpecialRuntimeObjects = new SpecialRuntimeObjects();
	return theSpecialRuntimeObjects;
}

SpecialRuntimeObjects::SpecialRuntimeObjects()
{
	createSpecialObjectTable();
	createSpecialClassTable();
}

SpecialRuntimeObjects::~SpecialRuntimeObjects()
{
}

void SpecialRuntimeObjects::createSpecialObjectTable()
{
    WithoutGC wgc;

	specialObjectTable.push_back(nilOop());
	specialObjectTable.push_back(trueOop());
	specialObjectTable.push_back(falseOop());

    // Block activation selectors
    blockActivationSelectorFirst = specialObjectTable.size();
    specialObjectTable.push_back(makeSelector("value"));
    specialObjectTable.push_back(makeSelector("value:"));
    specialObjectTable.push_back(makeSelector("value:value:"));
    specialObjectTable.push_back(makeSelector("value:value:value:"));
    specialObjectTable.push_back(makeSelector("value:value:value:value:"));
    specialObjectTable.push_back(makeSelector("value:value:value:value:value:"));
    specialObjectTable.push_back(makeSelector("value:value:value:value:value:value:"));
    blockActivationSelectorCount = specialObjectTable.size() - blockActivationSelectorFirst;
}

void SpecialRuntimeObjects::createSpecialClassTable()
{
#define SPECIAL_CLASS_NAME(className) \
	specialClassTable.push_back(className::ClassObject); \
	specialClassTable.push_back(className::MetaclassObject);
#include "SpecialClasses.inc"
#undef SPECIAL_CLASS_NAME
}

// Garbage collector
class GarbageCollector
{
public:
	enum Color
	{
		White = 0,
		Gray,
		Black,
	};

	GarbageCollector();
	~GarbageCollector();

    void initialize();

	uint8_t *allocateObjectMemory(size_t objectSize);

	void performCollection();

	void registerOopReference(OopRef *ref);
	void unregisterOopReference(OopRef *ref);

	void registerGCRoot(Oop *gcroot, size_t size);
	void unregisterGCRoot(Oop *gcroot);

    void enable();
    void disable();

private:
	template<typename FT>
	void onRootsDo(const FT &f)
	{
		// Traverse the root pointers
		for(auto &rootSize : rootPointers)
		{
			auto roots = rootSize.first;
			auto size = rootSize.second;
			for(size_t i = 0; i < size; ++i)
				f(roots[i]);
		}

		// Traverse the stacks
		for(auto stack : currentStacks)
		{
			stack->stackFramesDo([&](StackFrame &stackFrame) {
				stackFrame.oopElementsDo(f);
			});
		}

		// Traverse the oop reference
		OopRef *pos = firstReference;
		for(; pos; pos = pos->nextReference_)
		{
			f(pos->oop);
		}
	}

	void mark();
	void markObject(Oop objectPointer);
	void sweep();

	std::mutex controlMutex;
	std::vector<std::pair<Oop*, size_t>> rootPointers;
	std::list<Oop> allocatedObjects;
	std::vector<StackMemory*> currentStacks;
	OopRef *firstReference;
	OopRef *lastReference;
    bool enabled;
};

static GarbageCollector *theGarbageCollector = nullptr;

static GarbageCollector *getGC()
{
	if(!theGarbageCollector)
    {
		theGarbageCollector = new GarbageCollector();
        theGarbageCollector->initialize();
    }
	return theGarbageCollector;
}

GarbageCollector::GarbageCollector()
	: firstReference(nullptr), lastReference(nullptr), enabled(true)
{
}

GarbageCollector::~GarbageCollector()
{
}

void GarbageCollector::initialize()
{
    auto specialObjects = getSpecialRuntimeObjects();

	// Register the special objects table.
	registerGCRoot(&specialObjects->specialObjectTable[0], specialObjects->specialObjectTable.size());

	// Register the special classes table.
	registerGCRoot((Oop*)&specialObjects->specialClassTable[0], specialObjects->specialClassTable.size());
}

void GarbageCollector::enable()
{
    std::unique_lock<std::mutex> l(controlMutex);
    enabled = true;
}

void GarbageCollector::disable()
{
    std::unique_lock<std::mutex> l(controlMutex);
    enabled = false;
}

uint8_t *GarbageCollector::allocateObjectMemory(size_t objectSize)
{
	performCollection();
	assert(objectSize >= sizeof(ObjectHeader));

	// TODO: use a proper GCed heap.
	auto result = (uint8_t*)malloc(objectSize);
	auto header = reinterpret_cast<ObjectHeader*> (result);
	*header = {0};

	allocatedObjects.push_back(Oop::fromPointer(result));
	return result;
}

void GarbageCollector::registerOopReference(OopRef *ref)
{
	std::unique_lock<std::mutex> l(controlMutex);
	assert(ref);

	// Insert the reference into the beginning doubly linked list.
	ref->prevReference_ = nullptr;
	ref->nextReference_ = firstReference;
	if(firstReference)
		firstReference->prevReference_ = ref;
	firstReference = ref;

	// Make sure the last reference is set.
	if(!lastReference)
		lastReference = firstReference;
}

void GarbageCollector::unregisterOopReference(OopRef *ref)
{
	std::unique_lock<std::mutex> l(controlMutex);
	assert(ref);

	// Remove the reference from the double linked list.
	if(ref->prevReference_)
		ref->prevReference_->nextReference_ = ref->nextReference_;
	if(ref->nextReference_)
		ref->nextReference_->prevReference_ = ref->prevReference_;

	// Check the beginning.
	if(!ref->prevReference_)
		firstReference = ref->nextReference_;

	// Check the tail.
	if(!ref->nextReference_)
		lastReference = ref->prevReference_;
}

void GarbageCollector::registerGCRoot(Oop *gcroot, size_t size)
{
	std::unique_lock<std::mutex> l(controlMutex);
	rootPointers.push_back(std::make_pair(gcroot, size));
}

void GarbageCollector::unregisterGCRoot(Oop *gcroot)
{
	std::unique_lock<std::mutex> l(controlMutex);
	for(size_t i = 0; i < rootPointers.size(); ++i)
	{
		if(rootPointers[i].first == gcroot)
			rootPointers.erase(rootPointers.begin() + i);
	}
}

void GarbageCollector::performCollection()
{
	std::unique_lock<std::mutex> l(controlMutex);
    if(!enabled)
        return;

	// Get the current stacks
	currentStacks = getAllStackMemories();

	// TODO: Suspend the other GC threads.
	mark();
	sweep();
}

void GarbageCollector::mark()
{
	// Mark from the root objects.
	onRootsDo([this](Oop root) {
		markObject(root);
	});
}

void GarbageCollector::markObject(Oop objectPointer)
{
	// TODO: Use the schorr-waite algorithm.

	// mark pointer objects.
	if(!objectPointer.isPointer())
		return;

	// Get the object header.
	auto header = reinterpret_cast<ObjectHeader*> (objectPointer.pointer);
	if(header->gcColor)
		return;

	// Mark gray
	header->gcColor = Gray;

	// Mark recursively the children
	auto format = header->objectFormat;
	if(format == OF_FIXED_SIZE ||
	   format == OF_VARIABLE_SIZE_NO_IVARS ||
	   format == OF_VARIABLE_SIZE_IVARS)
	{
		auto slotCount = header->slotCount;
		auto headerSize = sizeof(ObjectHeader);
		if(slotCount == 255)
		{
			auto bigHeader = reinterpret_cast<BigObjectHeader*> (header);
			slotCount = bigHeader->slotCount;
			headerSize += 8;
		}

		// Traverse the slots.
		auto slots = reinterpret_cast<Oop*> (objectPointer.pointer + headerSize);
		for(size_t i = 0; i < slotCount; ++i)
			markObject(slots[i]);
	}

	// Special handilng of compiled method literals
	if(format >= OF_COMPILED_METHOD)
	{
		auto compiledMethod = reinterpret_cast<CompiledMethod*> (objectPointer.pointer);
		auto literalCount = compiledMethod->getLiteralCount();
		auto literals = compiledMethod->getFirstLiteralPointer();
		for(size_t i = 0; i < literalCount; ++i)
			markObject(literals[i]);
	}

	// Mark as black before ending.
	header->gcColor = Black;
}

void GarbageCollector::sweep()
{
	// Sweep the allocated objects.
	auto it = allocatedObjects.begin();
	for(; it != allocatedObjects.end(); )
	{
		auto &obj = *it;
		auto header = reinterpret_cast<ObjectHeader*> (obj.pointer);
		if(header->gcColor == White)
		{
			// TODO: free the unreachable object.
			//printf("free garbage %p %s\n", header, getClassNameOfObject(obj).c_str());
			fflush(stdout);
			free(header);
			allocatedObjects.erase(it++);
		}
		else
		{
			header->gcColor = White;
			++it;
		}
	}

	// Some roots were not allocated by myself. Clear their marks
	onRootsDo([this](Oop root) {
		if(!root.isPointer())
			return;

		auto header = reinterpret_cast<ObjectHeader*> (root.pointer);
		header->gcColor = White;
	});

}


// OopRef
void OopRef::registerSelf()
{
	getGC()->registerOopReference(this);
}

void OopRef::unregisterSelf()
{
	getGC()->unregisterOopReference(this);
}

// GC collector public interface
void disableGC()
{
    getGC()->disable();
}

void enableGC()
{
    getGC()->enable();
}

void registerGCRoot(Oop *gcroot, size_t size)
{
	getGC()->registerGCRoot(gcroot, size);
}

void unregisterGCRoot(Oop *gcroot)
{
	getGC()->unregisterGCRoot(gcroot);
}

uint8_t *allocateObjectMemory(size_t objectSize)
{
	return getGC()->allocateObjectMemory(objectSize);
}

ObjectHeader *newObject(size_t fixedSlotCount, size_t indexableSize, ObjectFormat format, int classIndex, int identityHash)
{
	// Compute the header size.
	size_t indexableSlotCount = 0;
	auto indexableSlotSize = variableSlotSizeFor(format);
	auto indexableFormatExtraBits = 0;
	bool hasPrimitiveData = false;
	if(indexableSlotSize && indexableSize)
	{
		if(format < OF_INDEXABLE_NATIVE_FIRST)
		{
			indexableSlotCount = indexableSize;
		}
#ifndef OBJECT_MODEL_SPUR_64
		if(format == OF_INDEXABLE_64)
		{
			indexableSlotCount = indexableSize * 2;
			hasPrimitiveData = true;
		}
#endif
		else
		{
			hasPrimitiveData = true;
			size_t divisor = variableSlotDivisor(format);
			size_t mask = (divisor - 1);

		 	indexableSlotCount = ((indexableSize + divisor - 1) & (~mask)) / divisor;
			indexableFormatExtraBits = indexableSize & mask;
			assert(indexableSize <= indexableSlotCount*divisor);
		}
	}

	// Compute more sizes
	auto totalSlotCount = fixedSlotCount + indexableSlotCount;
	auto headerSize = sizeof(ObjectHeader);
	if(totalSlotCount >= 255)
		headerSize += 8;
	auto fixedSlotSize = fixedSlotCount * sizeof(void*);
	auto variableSlotSize = indexableSlotCount * sizeof(void*);
	auto bodySize = fixedSlotSize + variableSlotSize;
	auto objectSize = headerSize + bodySize;

	// Allocate the object memory
	auto data = allocateObjectMemory(objectSize);
	auto header = reinterpret_cast<ObjectHeader*> (data);

	// Generate a hash if requested.
	if(identityHash < 0)
		identityHash = generateIdentityHash(data);

	// Set the object header.
	*header = {0};
	header->slotCount = totalSlotCount < 255 ? totalSlotCount : 255;
	header->identityHash = identityHash;
	header->objectFormat = format + indexableFormatExtraBits;
	header->classIndex = classIndex;
	if(totalSlotCount >= 255)
	{
		auto bigHeader = reinterpret_cast<BigObjectHeader*> (header);
		bigHeader->slotCount = totalSlotCount;
	}

	// Initialize the slots.
	auto slotStarts = data + headerSize;
	if(!hasPrimitiveData)
	{
		auto slots = reinterpret_cast<Object**> (slotStarts);
		for(size_t i = 0; i < totalSlotCount; ++i)
			slots[i] = &NilObject;
	}
	else
	{
		memset(slotStarts, 0, bodySize);
	}

	// Return the object.
	return header;
}

// Object creation / accessing
Ref<ProtoObject> makeIntegerObject(int value)
{
	// TODO: Check the small integer range
	return Ref<ProtoObject>::fromOop(Oop::encodeSmallInteger(value));
}

Ref<ProtoObject> makeFloatObject(double value)
{
	// TODO: Implement this
	return makeRef(&NilObject);
}

// Get a class from its index
Oop getClassFromIndex(int classIndex)
{
	// TODO: Use a proper class index manager
	auto specialObjects = getSpecialRuntimeObjects();
	if((size_t)classIndex >= specialObjects->specialClassTable.size())
	{
		LODTALK_UNIMPLEMENTED();
	}

	return Oop::fromPointer(specialObjects->specialClassTable[classIndex]);
}

Oop getClassFromOop(Oop oop)
{
	return getClassFromIndex(classIndexOf(oop));
}


bool isClassOrMetaclass(Oop oop)
{
	auto classIndex = classIndexOf(oop);
	if(classIndex == SCI_Metaclass)
		return true;

	auto clazz = getClassFromIndex(classIndexOf(oop));
	return classIndexOf(clazz) == SCI_Metaclass;
}

bool isMetaclass(Oop oop)
{
	return classIndexOf(oop) == SCI_Metaclass;
}

bool isClass(Oop oop)
{
	return isClassOrMetaclass(oop) && !isMetaclass(oop);
}

// Send the message
Oop sendDNUMessage(Oop receiver, Oop selector, int argumentCount, Oop *arguments)
{
	printf("TODO: Send DNU message\n");
	abort();
}

Oop lookupMessage(Oop receiver, Oop selector)
{
	auto classIndex = classIndexOf(receiver);
	auto classOop = getClassFromIndex(classIndex);
	assert(!isNil(classOop));

	// Lookup the method
	auto behavior = reinterpret_cast<Behavior*> (classOop.pointer);
	return behavior->lookupSelector(selector);
}

Oop sendMessage(Oop receiver, Oop selector, int argumentCount, Oop *arguments)
{
	auto method = lookupMessage(receiver, selector);
	if(isNil(method))
		return sendDNUMessage(receiver, selector, argumentCount, arguments);

	// Execute the method
	auto methodClassIndex = classIndexOf(method);
	if(methodClassIndex == SCI_CompiledMethod)
	{
		auto compiledMethod = reinterpret_cast<CompiledMethod*> (method.pointer);
		return compiledMethod->execute(receiver, argumentCount, arguments);
	}
	else if(methodClassIndex == SCI_NativeMethod)
	{
		auto nativeMethod = reinterpret_cast<NativeMethod*> (method.pointer);
		return nativeMethod->execute(receiver, argumentCount, arguments);
	}
	else
	{
		printf("TODO: Send run:with:in:\n");
		abort();
	}
}

Oop makeByteString(const std::string &content)
{
	return ByteString::fromNative(content).getOop();
}

Oop makeByteSymbol(const std::string &content)
{
	return ByteSymbol::fromNative(content).getOop();
}

Oop makeSelector(const std::string &content)
{
	return makeByteSymbol(content);
}

Oop sendBasicNew(Oop clazz)
{
	return sendMessage(clazz, makeSelector("basicNew"), 0, nullptr);
}

Oop sendBasicNewWithSize(Oop clazz, size_t size)
{
	Oop sizeOop = Oop::encodeSmallInteger(size);
	return sendMessage(clazz, makeSelector("basicNew:"), 1, &sizeOop);
}

// Global dictionary
static SystemDictionary *theGlobalDictionary = nullptr;
static SystemDictionary *getGlobalDictionary()
{
	if(!theGlobalDictionary)
	{
		theGlobalDictionary = new SystemDictionary();
		registerGCRoot((Oop*)&theGlobalDictionary, 1);
	}

	return theGlobalDictionary;
}

Oop setGlobalVariable(const char *name, Oop value)
{
	return setGlobalVariable(ByteSymbol::fromNative(name).getOop(), value);
}

Oop setGlobalVariable(Oop symbol, Oop value)
{
	auto globalDictionary = getGlobalDictionary();

	// Set the existing value.
	auto globalVar = globalDictionary->getNativeAssociationOrNil(symbol);
	if(classIndexOf(Oop::fromPointer(globalVar)) == SCI_GlobalVariable)
	{
		globalVar->value = value;
		return Oop::fromPointer(globalVar);
	}

	// Create the global variable
	Ref<Association> newGlobalVar = GlobalVariable::make(symbol, value);
	globalDictionary->putNativeAssociation(newGlobalVar.get());
	return newGlobalVar.getOop();
}

Oop getGlobalFromName(const char *name)
{
	return getGlobalFromName(name);
}

Oop getGlobalFromSymbol(Oop symbol)
{
	return Oop::fromPointer(getGlobalDictionary()->getNativeAssociationOrNil(symbol));
}

Oop getGlobalValueFromName(const char *name)
{
	return getGlobalValueFromSymbol(ByteSymbol::fromNative(name).getOop());
}

Oop getGlobalValueFromSymbol(Oop symbol)
{
	auto globalDictionary = getGlobalDictionary();
	auto globalVar = globalDictionary->getNativeAssociationOrNil(symbol);
	if(classIndexOf(Oop::fromPointer(globalVar)) != SCI_GlobalVariable)
		return nilOop();
	return globalVar->value;
}

Oop getGlobalContext()
{
	return Oop::fromPointer(GlobalContext::ClassObject);
}

// Object reading
std::string getClassNameOfObject(Oop object)
{
	auto classIndex = classIndexOf(object);
	auto classOop = getClassFromIndex(classIndex);
	if(classOop.header->classIndex == SCI_Metaclass)
		return "a Class";

	auto clazz = reinterpret_cast<Class*> (classOop.pointer);
	return clazz->getNameString();
}

std::string getByteSymbolData(Oop object)
{
	auto symbol = reinterpret_cast<ByteSymbol*> (object.pointer);
	return symbol->getString();
}

std::string getByteStringData(Oop object)
{
	auto string = reinterpret_cast<ByteString*> (object.pointer);
	return string->getString();
}

// Special selectors.
Oop getBlockActivationSelector(size_t argumentCount)
{
    auto specialObjects = getSpecialRuntimeObjects();
    if(argumentCount >= specialObjects->blockActivationSelectorCount)
        return Oop();
    return specialObjects->specialObjectTable[specialObjects->blockActivationSelectorFirst + argumentCount];
}
} // End of namespace Lodtalk
