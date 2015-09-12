#ifndef METHOD_BUILDER_HPP
#define METHOD_BUILDER_HPP

#include "Object.hpp"
#include "Collections.hpp"
#include "Method.hpp"

namespace Lodtalk
{
namespace MethodAssembler
{

/**
 * Method assembler node.
 */
class InstructionNode
{
public:
	InstructionNode()
		: position(0) {}
	virtual ~InstructionNode() {}
	
	virtual bool isReturnInstruction() const
	{
		return false;
	} 
	
	size_t getPosition()
	{
		return position;
	}
	
	size_t computeMaxSizeForPosition(size_t newPosition)
	{
		position = newPosition;
		return size = computeMaxSize();
	}
	
	size_t computeBetterSizeForPosition(size_t newPosition)
	{
		position = newPosition;
		return size = computeBetterSize();
	}
	
	virtual uint8_t *encode(uint8_t *buffer) = 0;
	
protected:
	virtual size_t computeMaxSize() = 0;
	virtual size_t computeBetterSize()
	{
		return computeMaxSize();
	}
	
private:
	size_t position;
	size_t size;
};

class Label: public InstructionNode
{
public:
	virtual uint8_t *encode(uint8_t *buffer)
	{
		return buffer;
	}
	
protected:
	virtual size_t computeMaxSize()
	{
		return 0;
	}
};

/**
 * Compiled method builder
 */
class Assembler
{
public:
	Assembler();
	~Assembler();

	void addInstruction(InstructionNode *instruction);
	size_t addLiteral(Oop newLiteral);
	size_t addLiteral(const OopRef &newLiteral);

	Label *makeLabel();
	Label *makeLabelHere();
	void putLabel(Label *label);

	bool isLastReturn();
	
	CompiledMethod *generate(size_t temporalCount, size_t argumentCount, size_t extraSize = 0);
	
public:
	void returnReceiver();
	void returnTrue();
	void returnFalse();
	void returnNil();
	void returnTop();
	
	void popStackTop();
	void duplicateStackTop();
	void pushLiteral(Oop literal);
	void pushLiteralVariable(Oop literalVariable);
	
	void pushReceiverVariableIndex(int variableIndex);
	void pushLiteralIndex(int literalIndex);
	void pushLiteralVariableIndex(int literalVariableIndex);
	void pushTemporal(int temporalIndex);

	void send(Oop selector, int argumentCount);
	void superSend(Oop selector, int argumentCount);
	
private:
	size_t computeInstructionsSize();
	
	std::vector<OopRef> literals;
	std::vector<InstructionNode*> instructionStream;	
};

} // End of namespace Method assembler
} // End of namespace Lodtalk

#endif //METHOD_BUILDER_HPP