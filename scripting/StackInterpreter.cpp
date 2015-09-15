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
	void interpret();

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
	
	int fetchByte()
	{
		return getInstructionBasePointer()[pc++];
	}

	int fetchSByte()
	{
		return reinterpret_cast<int8_t*> (getInstructionBasePointer())[pc++];
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
	
	void pushOop(Oop object)
	{
		stack->pushOop(object);
	}
	
	void pushPointer(uint8_t *pointer)
	{
		stack->pushPointer(pointer);
	}
	
	void pushUInt(uintptr_t value)
	{
		stack->pushUInt(value);
	}
	
	void pushPC()
	{
		pushUInt(pc);
	}
	
	void popPC()
	{
		pc = popUInt();
	}
	uint8_t *getInstructionBasePointer()
	{
		return reinterpret_cast<uint8_t*> (method);
	}
	
	Oop currentReceiver() const
	{
		return stack->getReceiver();
	}
	
	Oop popOop()
	{
		return stack->popOop();
	}

	uint8_t *popPointer()
	{
		return stack->popPointer();
	}
	
	uintptr_t popUInt()
	{
		return stack->popUInt();
	}
	
	Oop stackTop()
	{
		return stack->stackTop();
	}

	Oop stackOopAt(size_t offset)
	{
		return stack->stackOopAt(offset);
	}
	
	bool condJumpOnNotBoolean(bool jumpType)
	{
		LODTALK_UNIMPLEMENTED();
	}
	
	void nonLocalReturnValue(Oop value)
	{
		LODTALK_UNIMPLEMENTED();
	}
	
	void returnValue(Oop value)
	{
		// Special handling for non-local returns. 
		if(isBlock)
			nonLocalReturnValue(value);
			
		// Restore the stack into beginning of the frame pointer.
		stack->setStackPointer(stack->getFramePointer()); 
		stack->setFramePointer(stack->popPointer());
		
		// Pop the return pc
		popPC();
		
		// Pop the arguments and the receiver.
		stack->popMultiplesOops(argumentCount + 1);
		
		// Push the return value.
		pushOop(value);
		
		// If there is no, then it means that we are returning.
		if(pc)
		{
			// Re fetch the frame data to continue.
			fetchFrameData();
			
			// Fetch the next instruction
			fetchNextInstructionOpcode();
		}
	}
	
	void blockReturnValue(Oop value)
	{
		LODTALK_UNIMPLEMENTED();
	}
	
	void activateMethodFrame(CompiledMethod *method);
	void fetchFrameData();
	
private:
	// Use the stack memory.
	StackMemory *stack;
 
	// Interpreter data.
	size_t pc;
	int nextOpcode;
	int currentOpcode;

	// Instruction decoding.
	uint64_t extendA;
	int64_t extendB;

	// Frame meta data.
	Oop *literalArray;
	CompiledMethod *method;
	bool hasContext;
	bool isBlock;
	size_t argumentCount;
	

private:
	CompiledMethod *getMethod()
	{
		return method;
	}
	
	Oop getInstanceVariable(int index)
	{
		return reinterpret_cast<Oop*> (currentReceiver().getFirstFieldPointer())[index];
	}
	
	void setInstanceVariable(int index, Oop value)
	{
		reinterpret_cast<Oop*> (currentReceiver().getFirstFieldPointer())[index] = value;
	}
	
	Oop getLiteral(int index)
	{
		return literalArray[index];
	}

	ptrdiff_t getTemporaryOffset(size_t index)
	{
		if(index < argumentCount)
			return InterpreterStackFrame::LastArgumentOffset + ptrdiff_t(argumentCount - index - 1) *sizeof (Oop);
		else
			return InterpreterStackFrame::FirstTempOffset - ptrdiff_t(index - argumentCount) *sizeof (Oop);
	}
	
	Oop getTemporary(size_t index)
	{
		return *reinterpret_cast<Oop*> (stack->getFramePointer() + getTemporaryOffset(index));
	}

	void setTemporary(size_t index, Oop value)
	{
		*reinterpret_cast<Oop*> (stack->getFramePointer() + getTemporaryOffset(index)) = value;
	}
	
	void pushReceiverVariable(int receiverVarIndex)
	{
		auto localVar = getInstanceVariable(receiverVarIndex);
		pushOop(localVar);
	}
	
	void pushLiteralVariable(int literalVarIndex)
	{
		auto literal = getLiteral(literalVarIndex);
		
		// Cast the literal variable and push the value
		auto literalVar = reinterpret_cast<LiteralVariable*> (literal.pointer);
		pushOop(literalVar->value);
	}

	void pushLiteral(int literalIndex)
	{
		auto literal = getLiteral(literalIndex);
		pushOop(literal);
	}
	
	void pushTemporary(int temporaryIndex)
	{
		auto temporary = getTemporary(temporaryIndex);
		pushOop(temporary);
	}
	
	void sendLiteralIndexArgumentCount(int literalIndex, int argumentCount)
	{
		auto selector = getLiteral(literalIndex);
		assert((size_t)argumentCount <= CompiledMethodHeader::ArgumentMask);

		// Get the receiver.
		auto newReceiver = stack->stackOopAt(argumentCount * sizeof(Oop));
		//printf("Send #%s [%s]%p\n", getByteSymbolData(selector).c_str(), getClassNameOfObject(newReceiver).c_str(), newReceiver.pointer);
		
		// Find the called method
		auto calledMethodOop = lookupMessage(newReceiver, selector);
		if(calledMethodOop.isNil())
		{
			// TODO: Send a DNU
			LODTALK_UNIMPLEMENTED();
		}
		
		// Get the called method type
		auto methodClassIndex = classIndexOf(calledMethodOop);
		if(methodClassIndex == SCI_CompiledMethod)
		{
			// Push the return PC.
			pushPC();
			
			// Activate the new compiled method.
			auto compiledMethod = reinterpret_cast<CompiledMethod*> (calledMethodOop.pointer);
			activateMethodFrame(compiledMethod);
		}
		else if(methodClassIndex == SCI_NativeMethod)
		{
			// Reverse the argument order.
			Oop nativeMethodArgs[CompiledMethodHeader::ArgumentMask];
			for(int i = 0; i < argumentCount; ++i)
				nativeMethodArgs[argumentCount - i - 1] = stack->stackOopAt(i*sizeof(Oop));
			
			// Call the native method
			auto nativeMethod = reinterpret_cast<NativeMethod*> (calledMethodOop.pointer);
			Oop result = nativeMethod->execute(newReceiver, argumentCount, nativeMethodArgs);
			
			// Pop the arguments and the receiver.
			stack->popMultiplesOops(argumentCount + 1);
			
			// Push the result in the stack.
			pushOop(result);
			
			// Re fetch the frame data to continue.
			fetchFrameData();
			
			// Fetch the next instruction
			fetchNextInstructionOpcode();
		}
		else
		{
			// TODO: Send Send run:with:in:
			LODTALK_UNIMPLEMENTED();
		}
	}
	
	// Bytecode instructions
	void interpretPushReceiverVariableShort()
	{
		fetchNextInstructionOpcode();
		
		// Fetch the variable index.
		auto variableIndex = currentOpcode & 0xF;
		pushReceiverVariable(variableIndex);
	}
	
	void interpretPushLiteralVariableShort()
	{
		fetchNextInstructionOpcode();
		
		// Fetch the literal index
		auto literalVarIndex = currentOpcode & 0xF;
		pushLiteralVariable(literalVarIndex);
	}
	
	void interpretPushLiteralShort()
	{
		fetchNextInstructionOpcode();
		
		// Fetch the literal index
		auto literalIndex = currentOpcode & 0x1F; 
		pushLiteral(literalIndex);
	}
	
	void interpretPushTempShort()
	{
		fetchNextInstructionOpcode();
		
		// Fetch the temporal index
		auto tempIndex = currentOpcode - BytecodeSet::PushTempShortFirst;
		pushTemporary(tempIndex);
	}
	
	void interpretSendShortArgs0()
	{
		// Fetch the literal index
		auto literalIndex = currentOpcode & 0x0F;
		sendLiteralIndexArgumentCount(literalIndex, 0);
	}
	
	void interpretSendShortArgs1()
	{
		// Fetch the literal index
		auto literalIndex = currentOpcode & 0x0F;
		sendLiteralIndexArgumentCount(literalIndex, 1);
	}
	
	void interpretSendShortArgs2()
	{
		// Fetch the literal index
		auto literalIndex = currentOpcode & 0x0F;
		sendLiteralIndexArgumentCount(literalIndex, 2);
	}
	
	void interpretJumpShort()
	{
		auto delta = (currentOpcode & 7) + 1;
		pc += delta;
	}
	
	void interpretJumpOnTrueShort()
	{
		// Fetch the condition and the next instruction opcode
		fetchNextInstructionOpcode();
		auto condition = popOop();
		
		// Perform the branch when requested.
		if(condition == trueOop())
		{
			auto delta = (currentOpcode & 7);
			pc += delta;
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
		auto condition = popOop();
		
		// Perform the branch when requested.
		if(condition == falseOop())
		{
			auto delta = (currentOpcode & 7);
			pc += delta;
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
		pushOop(currentReceiver());
	}

	void interpretPushTrue()
	{
		fetchNextInstructionOpcode();
		pushOop(trueOop());
	}

	void interpretPushFalse()
	{
		fetchNextInstructionOpcode();
		pushOop(falseOop());
	}
	
	void interpretPushNil()
	{
		fetchNextInstructionOpcode();
		pushOop(nilOop());
	}
	
	void interpretPushZero()
	{
		fetchNextInstructionOpcode();
		pushOop(Oop::encodeSmallInteger(0));
	}
	
	void interpretPushOne()
	{
		fetchNextInstructionOpcode();
		pushOop(Oop::encodeSmallInteger(1));
	}

	void interpretPushThisContext()
	{
		LODTALK_UNIMPLEMENTED();
	}

	void interpretDuplicateStackTop()
	{
		fetchNextInstructionOpcode();
		pushOop(stackTop());
	}

	void interpretReturnReceiver()
	{
		returnValue(currentReceiver());
	}

	void interpretReturnTrue()
	{
		returnValue(trueOop());
	}
	
	void interpretReturnFalse()
	{
		returnValue(falseOop());
	}
	
	void interpretReturnNil()
	{
		returnValue(nilOop());
	}

	void interpretReturnTop()
	{
		returnValue(popOop());
	}

	void interpretBlockReturnNil()
	{
		blockReturnValue(nilOop());
	}

	void interpretBlockReturnTop()
	{
		blockReturnValue(popOop());
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
		popOop();
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
		auto tempIndex = fetchByte();
		fetchNextInstructionOpcode();
		
		pushTemporary(tempIndex);
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

	// Push the receiver and the arguments
	pushOop(receiver);
	for(int i = 0; i < argumentCount; ++i)
		pushOop(arguments[i]);

	// Make the method frame.
	pushUInt(0); // Return instruction PC.
	activateMethodFrame(newMethod);
	
	interpret();
	
	auto returnValue = popOop();
	return returnValue;
}

void StackInterpreter::activateMethodFrame(CompiledMethod *newMethod)
{
	// Get the method header
	auto header = *newMethod->getHeader();
	auto numArguments = header.getArgumentCount();
	auto numTemporals = header.getTemporalCount();
	
	// Get the receiver
	auto receiver = stackOopAt((1 + numArguments)*sizeof(Oop));

	// Push the frame pointer.
	pushPointer(stack->getFramePointer()); // Return frame pointer.

	// Set the new frame pointer.	
	stack->setFramePointer(stack->getStackPointer());
	
	// Push the method object.
	pushOop(Oop::fromPointer(newMethod));
	this->method = newMethod;
	
	// Encode frame metadata
	pushUInt(encodeFrameMetaData(false, false, numArguments));
	
	// Push the nil this context.
	pushOop(Oop()); 

	// Push the oop.
	pushOop(receiver);
	
	// Push the nil temporals.
	for(size_t i = 0; i < numTemporals; ++i)
		pushOop(Oop());

	// Fetch the frame data.
	fetchFrameData();
	
	// Set the instruction pointer. 
	pc = newMethod->getFirstPCOffset();
	
	// Fetch the first instruction opcode
	fetchNextInstructionOpcode();
}

void StackInterpreter::fetchFrameData()
{
	// Decode the frame metadata.
	decodeFrameMetaData(stack->getMetadata(), this->hasContext, this->isBlock, argumentCount);

	// Get the method and the literal array	
	method = stack->getMethod();
	literalArray = method->getFirstLiteralPointer();
}

void StackInterpreter::interpret()
{
	// Reset the extensions values
	extendA = 0;
	extendB = 0;
	while(pc != 0)
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
