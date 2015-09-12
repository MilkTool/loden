#include "MethodBuilder.hpp"
#include "BytecodeSets.hpp"

namespace Lodtalk
{
namespace MethodAssembler
{

const size_t ExtensibleBytecodeSizeMax = 16;


// Single bytecode instruction
class SingleBytecodeInstruction: public InstructionNode
{
public:
	SingleBytecodeInstruction(int bc, bool isReturn = false)
		: bytecode(bc), isReturn(isReturn) {}

	virtual bool isReturnInstruction() const
	{
		return isReturn;
	} 
	
	virtual uint8_t *encode(uint8_t *buffer)
	{
		*buffer++ = bytecode;
		return buffer;
	}
	
protected:
	virtual size_t computeMaxSize()
	{
		return 1;
	}

private:
	int bytecode;
	bool isReturn;
};

// PushReceiverVariable
class PushReceiverVariable: public InstructionNode
{
public:
	PushReceiverVariable(int index)
		: index(index) {}

	virtual uint8_t *encode(uint8_t *buffer)
	{
		assert(0 && "unimplemented");
	}
	
protected:
	virtual size_t computeMaxSize()
	{
		assert(0 && "unimplemented");
	}

private:
	int index;	
};

// PushLiteral
class PushLiteral: public InstructionNode
{
public:
	PushLiteral(int index)
		: index(index) {}

	virtual uint8_t *encode(uint8_t *buffer)
	{
		if(index < BytecodeSet::PushLiteralShortRangeSize)
		{
			*buffer++ = BytecodeSet::PushLiteralShortFirst + index;
			return buffer;
		}
			
		assert(0 && "unimplemented");
	}
	
protected:
	virtual size_t computeMaxSize()
	{
		if(index < BytecodeSet::PushLiteralShortRangeSize)
			return 1;
		assert(0 && "unimplemented");
	}

private:
	int index;	
};

// PushLiteralVariable
class PushLiteralVariable: public InstructionNode
{
public:
	PushLiteralVariable(int index)
		: index(index) {}

	virtual uint8_t *encode(uint8_t *buffer)
	{
		if(index < BytecodeSet::PushLiteralVariableShortRangeSize)
		{
			*buffer++ = BytecodeSet::PushLiteralVariableShortFirst + index;
			return buffer;
		}

		assert(0 && "unimplemented");
	}
	
protected:
	virtual size_t computeMaxSize()
	{
		if(index < BytecodeSet::PushLiteralVariableShortRangeSize)
			return 1;
		assert(0 && "unimplemented");
	}

private:
	int index;	
};

// Push temporal
class PushTemporal: public InstructionNode
{
public:
	PushTemporal(int index)
		: index(index) {}

	virtual uint8_t *encode(uint8_t *buffer)
	{
		assert(0 && "unimplemented");
	}
	
protected:
	virtual size_t computeMaxSize()
	{
		assert(0 && "unimplemented");
	}

private:
	int index;	
};

// Send message
class SendMessage: public InstructionNode
{
public:
	SendMessage(int selectorIndex, int argumentCount)
		: selectorIndex(selectorIndex), argumentCount(argumentCount) {}

	virtual uint8_t *encode(uint8_t *buffer)
	{
		if(argumentCount == 0 && selectorIndex < BytecodeSet::SendShortArgs0RangeSize)
		{
			*buffer++ = BytecodeSet::SendShortArgs0First + selectorIndex;
			return buffer;
		}
		if(argumentCount == 1 && selectorIndex < BytecodeSet::SendShortArgs0RangeSize)
		{
			*buffer++ = BytecodeSet::SendShortArgs1First + selectorIndex;
			return buffer;
		}
		if(argumentCount == 2 && selectorIndex < BytecodeSet::SendShortArgs0RangeSize)
		{
			*buffer++ = BytecodeSet::SendShortArgs2First + selectorIndex;
			return buffer;
		}

		
		if(argumentCount > BytecodeSet::Send_ArgumentCountMask)
			assert(0 && "unimplemented");
			
		if(selectorIndex > BytecodeSet::Send_LiteralIndexMask)
			assert(0 && "unimplemented");

		*buffer++ = BytecodeSet::Send;
		*buffer++ = (argumentCount & BytecodeSet::Send_ArgumentCountMask) |
			((selectorIndex & BytecodeSet::Send_LiteralIndexMask) << BytecodeSet::Send_LiteralIndexShift);
		return buffer;
	}
	
protected:
	virtual size_t computeMaxSize()
	{
		if((argumentCount == 0 && selectorIndex < BytecodeSet::SendShortArgs0RangeSize) ||
			(argumentCount == 1 && selectorIndex < BytecodeSet::SendShortArgs1RangeSize) ||
			(argumentCount == 2 && selectorIndex < BytecodeSet::SendShortArgs2RangeSize))
			return 1;

		size_t count = 2;
		
		if(argumentCount > BytecodeSet::Send_ArgumentCountMask)
			assert(0 && "unimplemented");

 		if(selectorIndex > BytecodeSet::Send_LiteralIndexMask)
			assert(0 && "unimplemented");
		return count;		
	}

private:
	int selectorIndex;
	int argumentCount;
};

// Super Send message
class SuperSendMessage: public InstructionNode
{
public:
	SuperSendMessage(int selectorIndex, int argumentCount)
		: selectorIndex(selectorIndex), argumentCount(argumentCount) {}

	virtual uint8_t *encode(uint8_t *buffer)
	{
		assert(0 && "unimplemented");
	}
	
protected:
	virtual size_t computeMaxSize()
	{
		assert(0 && "unimplemented");
	}

private:
	int selectorIndex;
	int argumentCount;
};

// The assembler
Assembler::Assembler()
{
}

Assembler::~Assembler()
{
}

void Assembler::addInstruction(InstructionNode *instruction)
{
	instructionStream.push_back(instruction);
}

size_t Assembler::addLiteral(Oop newLiteral)
{
	for (size_t i = 0; i < literals.size(); ++i)
	{
		if(literals[i] == newLiteral)
			return i;
	}
	
	auto ret = literals.size();
	literals.push_back(newLiteral);
	return ret;
}

size_t Assembler::addLiteral(const OopRef &newLiteral)
{
	return addLiteral(newLiteral.oop);
}

Label *Assembler::makeLabel()
{
	return new Label();
}

Label *Assembler::makeLabelHere()
{
	auto label = makeLabel();
	putLabel(label);
	return label;
}

void Assembler::putLabel(Label *label)
{
	instructionStream.push_back(label);
}

size_t Assembler::computeInstructionsSize()
{
	// Compute the max size.
	size_t maxSize = 0;
	for(auto &instr : instructionStream)
		maxSize += instr->computeMaxSizeForPosition(maxSize);
		
	// Compute the optimal iteratively.
	size_t oldSize = maxSize;
	size_t currentSize = 0;
	do
	{
		oldSize = currentSize;
		currentSize = 0;
		for(auto &instr : instructionStream)
			currentSize += instr->computeBetterSizeForPosition(currentSize);
	} while(currentSize < oldSize);
	
	return currentSize;
}

CompiledMethod *Assembler::generate(size_t temporalCount, size_t argumentCount, size_t extraSize)
{
	// Compute the method sizes.
	auto instructionsSize = computeInstructionsSize();
	auto literalCount = literals.size();
	auto methodSize = literalCount*sizeof(void*) + instructionsSize + extraSize;
	
	// Create the method header
	auto methodHeader = CompiledMethodHeader::create(literalCount, temporalCount, argumentCount);
	
	// Create the compiled method
	auto compiledMethod = CompiledMethod::newMethodWithHeader(methodSize, methodHeader);
	
	// Set the compiled method literals
	auto literalData = compiledMethod->getFirstLiteralPointer();
	for(size_t i = 0; i < literals.size(); ++i)
		literalData[i] = literals[i].oop;
		
	// Encode the bytecode instructions.
	auto instructionBuffer = compiledMethod->getFirstBCPointer();
	auto instructionBufferEnd = instructionBuffer + instructionsSize;
	for(auto &instr : instructionStream)
	{
		instructionBuffer = instr->encode(instructionBuffer);
		if(instructionBuffer > instructionBufferEnd)
		{
			fprintf(stderr, "Fatal error: Memory corruption\n");
			abort();
		}
	}
	
	return compiledMethod;
}

bool Assembler::isLastReturn()
{
	return !instructionStream.empty() && instructionStream.back()->isReturnInstruction();
}

void Assembler::returnReceiver()
{
	addInstruction(new SingleBytecodeInstruction(BytecodeSet::ReturnReceiver, true));
}

void Assembler::returnTrue()
{
	addInstruction(new SingleBytecodeInstruction(BytecodeSet::ReturnTrue, true));
}

void Assembler::returnFalse()
{
	addInstruction(new SingleBytecodeInstruction(BytecodeSet::ReturnFalse, true));
}

void Assembler::returnNil()
{
	addInstruction(new SingleBytecodeInstruction(BytecodeSet::ReturnNil, true));
}

void Assembler::returnTop()
{
	addInstruction(new SingleBytecodeInstruction(BytecodeSet::ReturnTop, true));
}

void Assembler::popStackTop()
{
	addInstruction(new SingleBytecodeInstruction(BytecodeSet::PopStackTop));
}

void Assembler::duplicateStackTop()
{
	addInstruction(new SingleBytecodeInstruction(BytecodeSet::DuplicateStackTop));
}

void Assembler::pushLiteral(Oop literal)
{
	pushLiteralIndex(addLiteral(literal));
}

void Assembler::pushLiteralVariable(Oop literalVariable)
{
	pushLiteralVariableIndex(addLiteral(literalVariable));
}

void Assembler::pushReceiverVariableIndex(int variableIndex)
{
	addInstruction(new PushReceiverVariable(variableIndex));
}

void Assembler::pushLiteralIndex(int literalIndex)
{
	addInstruction(new PushLiteral(literalIndex));
}

void Assembler::pushLiteralVariableIndex(int literalVariableIndex)
{
	addInstruction(new PushLiteralVariable(literalVariableIndex));
}

void Assembler::pushTemporal(int temporalIndex)
{
	addInstruction(new PushTemporal(temporalIndex));
}

void Assembler::send(Oop selector, int argumentCount)
{
	addInstruction(new SendMessage(addLiteral(selector), argumentCount));
}

void Assembler::superSend(Oop selector, int argumentCount)
{
	addInstruction(new SuperSendMessage(addLiteral(selector), argumentCount));
}

} // End of namespace MethodAssembler
} // End of namespace Lodtalk
