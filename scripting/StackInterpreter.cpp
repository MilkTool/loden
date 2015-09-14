#include <stdarg.h>
#include "StackMemory.hpp"
#include "StackInterpreter.hpp"
#include "PreprocessorHacks.hpp"
#include "BytecodeSets.hpp"
#include "Common.hpp"

namespace Lodtalk
{

/**
 * The stack interpreter.
 * This stack interpreter is based in the arquitecture of squeak.
 */ 
class StackInterpreter
{
public:
	StackInterpreter(StackMemory *stack);
	~StackInterpreter();
	
	Oop interpretMethod(CompiledMethod *method, Oop receiver, int argumentCount, Oop *arguments);
	
private:
	void error(const char *message)
	{
		fprintf(stdout, "Interpreter error: %s\n", message);
		abort();
	}

	void errorFormat(const char *format, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, format);
		vsnprintf(buffer, 1024, format, args);
		error(buffer);
		abort();
	}
	
	void fetchMethodData(CompiledMethod *newMethod)
	{
		method = newMethod;
		literalCount = method->getLiteralCount();
		argumentCount = method->getArgumentCount();
		temporalCount = method->getTemporalCount();
		firstInstructionPointer = method->getFirstBCPointer();
		literalArray = method->getFirstLiteralPointer();
	}
	
	int fetchByte()
	{
		return *instructionPointer++;
	}

	int fetchSByte()
	{
		auto sbyte = *reinterpret_cast<int8_t*> (instructionPointer);
		instructionPointer++;
		return sbyte;
	}
	
	void fetchNextInstructionOpcode()
	{
		nextOpcode = fetchByte();
	}

	size_t getStackSize() const
	{
		return stack->getStackSize();
	}

	size_t getAvailableCapacity() const
	{
		return stack->getAvailableCapacity();
	}
	
	void push(Oop object)
	{
		if(!getAvailableCapacity())
			error("Stack overflow");
		stack->push(object);
	}
	
	Oop currentReceiver() const
	{
		return methodReceiver;
	}
	
	Oop pop()
	{
		if(!getStackSize())
			error("Stack underflow");
		return stack->pop();
	}
	
	Oop stackTop()
	{
		if(!getStackSize())
			error("Stack underflow");
		return stack->stackTop();
	}
	
	bool condJumpOnNotBoolean(bool jumpType)
	{
		LODTALK_UNIMPLEMENTED();
	}
	
private:
	// Use the stack memory.
	StackMemory *stack;
 
	// Interpreter data
	CompiledMethod *method;
	uint8_t *firstInstructionPointer;
	uint8_t *instructionPointer;
	int nextOpcode;
	int currentOpcode;
	
	size_t argumentCount;
	size_t literalCount;
	size_t temporalCount;
	
	uint64_t extendA;
	int64_t extendB;
	
	Oop methodReceiver;
	Oop *arguments;
	Oop *literalArray;
	
	bool isReturning;
	Oop methodReturnValue;

private:
	void pushLiteralVariable(int literalVarIndex)
	{
		auto literal = literalArray[literalVarIndex];
		
		// Cast the literal variable and push the value
		auto literalVar = reinterpret_cast<LiteralVariable*> (literal.pointer);
		push(literalVar->value);
	}

	void pushLiteral(int literalIndex)
	{
		auto literal = literalArray[literalIndex];
		
		// Cast the literal variable and push the value
		push(literal);
	}
	
	void sendLiteralIndexArgumentCount(int literalIndex, int argumentCount)
	{
		auto selector = literalArray[literalIndex];
		
		// TODO: Make this more efficient.
		Oop arguments[argumentCount];
		for(int i = 0; i < argumentCount; ++i)
			arguments[argumentCount - i - 1] = pop();

		// Get the receiver.
		Oop newReceiver = pop();
		
		// Perform the message send.
		Oop result = sendMessage(newReceiver, selector, argumentCount, arguments);
		push(result);
	}
	
	// Bytecode instructions
	void interpretPushReceiverVariableShort()
	{
		LODTALK_UNIMPLEMENTED();
	}
	
	void interpretPushLiteralVariableShort()
	{
		fetchNextInstructionOpcode();
		
		// Fetch the literal inde 
		auto literalVarIndex = currentOpcode & 0xF;
		pushLiteralVariable(literalVarIndex);
	}
	
	void interpretPushLiteralShort()
	{
		fetchNextInstructionOpcode();
		
		// Fetch the literal inded
		auto literalIndex = currentOpcode & 0x1F; 
		pushLiteral(literalIndex);
	}
	
	void interpretPushTempShort()
	{
		LODTALK_UNIMPLEMENTED();
	}
	
	void interpretSendShortArgs0()
	{
		fetchNextInstructionOpcode();
		
		// Fetch the literal index
		auto literalIndex = currentOpcode & 0x0F;
		sendLiteralIndexArgumentCount(literalIndex, 0);
	}
	
	void interpretSendShortArgs1()
	{
		fetchNextInstructionOpcode();
		
		// Fetch the literal index
		auto literalIndex = currentOpcode & 0x0F;
		sendLiteralIndexArgumentCount(literalIndex, 1);
	}
	
	void interpretSendShortArgs2()
	{
		fetchNextInstructionOpcode();
		
		// Fetch the literal index
		auto literalIndex = currentOpcode & 0x0F;
		sendLiteralIndexArgumentCount(literalIndex, 2);
	}
	
	void interpretJumpShort()
	{
		auto delta = (currentOpcode & 7) + 1;
		instructionPointer += delta;
	}
	
	void interpretJumpOnTrueShort()
	{
		// Fetch the condition and the next instruction opcode
		fetchNextInstructionOpcode();
		auto condition = pop();
		
		// Perform the branch when requested.
		if(condition == trueOop())
		{
			auto delta = (currentOpcode & 7);
			instructionPointer += delta;
			fetchNextInstructionOpcode();
		}
		else if(condition != falseOop())
		{
			// If the condition is not a boolean, trap
			condJumpOnNotBoolean(true);
		}
	}
	
	void interpretJumpOnFalseShort()
	{
		// Fetch the condition and the next instruction opcode
		fetchNextInstructionOpcode();
		auto condition = pop();
		
		// Perform the branch when requested.
		if(condition == falseOop())
		{
			auto delta = (currentOpcode & 7);
			instructionPointer += delta;
			fetchNextInstructionOpcode();
		}
		else if(condition != trueOop())
		{
			// If the condition is not a boolean, trap
			condJumpOnNotBoolean(false);
		}
	}
	
	void interpretPopStoreReceiverVariableShort()
	{
		LODTALK_UNIMPLEMENTED();
	}
	
	void interpretPopStoreTemporalVariableShort()
	{
		LODTALK_UNIMPLEMENTED();
	}
	
	void interpretPushReceiver()
	{
		fetchNextInstructionOpcode();
		push(currentReceiver());
	}

	void interpretPushTrue()
	{
		fetchNextInstructionOpcode();
		push(trueOop());
	}

	void interpretPushFalse()
	{
		fetchNextInstructionOpcode();
		push(falseOop());
	}
	
	void interpretPushNil()
	{
		fetchNextInstructionOpcode();
		push(nilOop());
	}
	
	void interpretPushZero()
	{
		fetchNextInstructionOpcode();
		push(Oop::encodeSmallInteger(0));
	}
	
	void interpretPushOne()
	{
		fetchNextInstructionOpcode();
		push(Oop::encodeSmallInteger(1));
	}

	void interpretPushThisContext()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretDuplicateStackTop()
	{
		fetchNextInstructionOpcode();
		push(stackTop());
	}

	void interpretReturnReceiver()
	{
		isReturning = true;
		methodReturnValue = currentReceiver();
	}

	void interpretReturnTrue()
	{
		isReturning = true;
		methodReturnValue = trueOop();
	}
	
	void interpretReturnFalse()
	{
		isReturning = true;
		methodReturnValue = falseOop();
	}
	
	void interpretReturnNil()
	{
		isReturning = true;
		methodReturnValue = nilOop();
	}

	void interpretReturnTop()
	{
		isReturning = true;
		methodReturnValue = pop();
	}

	void interpretBlockReturnNil()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretBlockReturnTop()
	{
		LODTALK_UNIMPLEMENTED();
	}
	
	void interpretNop()
	{
		fetchNextInstructionOpcode();
		
		// Do nothing
	}

	void interpretPopStackTop()
	{
		fetchNextInstructionOpcode();
		
		// Pop the element from the stack.
		pop();
	}

	void interpretExtendA()
	{
		extendA = extendA*256 + fetchByte();
		fetchNextInstructionOpcode();
	}

	void interpretExtendB()
	{
		extendB = extendB*256 + fetchSByte();
		fetchNextInstructionOpcode();
	}

	void interpretPushReceiverVariable()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretPushLiteralVariable()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretPushLiteral()
	{
		LODTALK_UNIMPLEMENTED();
	}
	
	void interpretPushTemporary()
	{
		LODTALK_UNIMPLEMENTED();
	}
	
	void interpretPushNTemps()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretPushInteger()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretPushCharacter()
	{
		LODTALK_UNIMPLEMENTED();
	}
	
	void interpretPushArrayWithElements()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretSend()
	{
		// Fetch the data.
		int data = fetchByte();
		fetchNextInstructionOpcode();
		
		// Decode the literal index and argument index
		auto argumentCount = (data & BytecodeSet::Send_ArgumentCountMask) + extendB * BytecodeSet::Send_ArgumentCountCount;
		auto literalIndex = ((data >> BytecodeSet::Send_LiteralIndexShift) & BytecodeSet::Send_LiteralIndexMask) + extendA * BytecodeSet::Send_LiteralIndexCount;
		
		// Clear the extension values.
		extendA = 0;
		extendB = 0;
		
		// Send the message
		sendLiteralIndexArgumentCount(literalIndex, argumentCount);
	}

	void interpretSuperSend()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretTrapOnBehavior()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretJump()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretJumpOnTrue()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretJumpOnFalse()
	{
		LODTALK_UNIMPLEMENTED();
	}
	
	void interpretPopStoreReceiverVariable()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretPopStoreLiteralVariable()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretPopStoreTemporalVariable()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretStoreReceiverVariable()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretStoreLiteralVariable()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretStoreTemporalVariable()
	{
		LODTALK_UNIMPLEMENTED();
	}

};

StackInterpreter::StackInterpreter(StackMemory *stack)
	: stack(stack)
{
}

StackInterpreter::~StackInterpreter()
{
}
	
Oop StackInterpreter::interpretMethod(CompiledMethod *newMethod, Oop receiver, int argumentCount, Oop *arguments)
{
	// Check the argument count
	if(argumentCount != (int)newMethod->getArgumentCount())
		error("invalid suplied argument count.");

	isReturning = false;
	
	// Store the receiver and the argument data
	this->methodReceiver = receiver;
	this->arguments = arguments;
	
	// Fetch the method data
	fetchMethodData(newMethod);
	instructionPointer = firstInstructionPointer;
	
	// Fetch the first instruction opcode
	fetchNextInstructionOpcode();
	
	// Reset the extensions values
	extendA = 0;
	extendB = 0;
	
	while(!isReturning)
	{
		currentOpcode = nextOpcode;
		
		switch(currentOpcode)
		{
#define MAKE_RANGE_CASE(i, from, to, name) \
case i + from:\
	interpret ## name(); \
	break;

#define SISTAV1_INSTRUCTION_RANGE(name, rangeFirst, rangeLast) \
	LODTALK_EVAL(LODTALK_FROM_TO_INCLUSIVE(rangeFirst, rangeLast, MAKE_RANGE_CASE, name))

#define SISTAV1_INSTRUCTION(name, opcode) \
case opcode:\
	interpret ## name(); \
	break;
#include "SistaV1BytecodeSet.inc"
#undef SISTAV1_INSTRUCTION_RANGE
#undef SISTAV1_INSTRUCTION
#undef MAKE_RANGE_CASE

		default:
			errorFormat("unsupported bytecode %d", currentOpcode);
		}
	}
	
	return methodReturnValue;
}
	
Oop interpretCompiledMethod(CompiledMethod *method, Oop receiver, int argumentCount, Oop *arguments)
{
	Oop result;
	withStackMemory([&](StackMemory *stack) {
		StackInterpreter interpreter(stack);
		result = interpreter.interpretMethod(method, receiver, argumentCount, arguments); 
	});
	return result;
}

} // End of namespace Lodtalk
