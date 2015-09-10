#include <memory>
#include <stdio.h>
#include <stdarg.h>
#include "Compiler.hpp"
#include "Method.hpp"
#include "ParserScannerInterface.hpp"

namespace Lodtalk
{

using namespace AST;
	
// Variable lookup description
class VariableLookup
{
public:
	VariableLookup() {}
	virtual ~VariableLookup() {}
	
	virtual bool isMutable() const = 0;

	virtual Oop getValue() = 0;
	virtual void setValue(Oop newValue) = 0;
};

typedef std::shared_ptr<VariableLookup> VariableLookupPtr;

// Evaluation scope class
class EvaluationScope;
typedef std::shared_ptr<EvaluationScope> EvaluationScopePtr;

class EvaluationScope
{
public:
	EvaluationScope(const EvaluationScopePtr &parentScope);
	virtual ~EvaluationScope();
	
	virtual VariableLookupPtr lookSymbol(Oop symbol) = 0;
	
	VariableLookupPtr lookSymbolRecursively(Oop symbol);
	
	const EvaluationScopePtr &getParent() const
	{
		return parentScope;
	}
	
private:
	EvaluationScopePtr parentScope;
};

EvaluationScope::EvaluationScope(const EvaluationScopePtr &parentScope)
	: parentScope(parentScope)
{
}

EvaluationScope::~EvaluationScope()
{
}

VariableLookupPtr EvaluationScope::lookSymbolRecursively(Oop symbol)
{
	auto result = lookSymbol(symbol);
	if(result || !parentScope)
		return result;
	return parentScope->lookSymbolRecursively(symbol);
}

// Literal variable lookup
class LiteralVariableLookup: public VariableLookup
{
public:
	LiteralVariableLookup(Oop literalVariable)
		: variable(reinterpret_cast<LiteralVariable*> (literalVariable.pointer)) {}
	~LiteralVariableLookup() {}
	
	virtual bool isMutable() const
	{
		return true;
	}

	virtual Oop getValue()
	{
		return variable->value;
	}
	
	virtual void setValue(Oop newValue)
	{
		variable->value = newValue;
	}

	LiteralVariable *variable;
};

// Global variable evaluation scope
class GlobalEvaluationScope: public EvaluationScope
{
public:
	GlobalEvaluationScope()
		: EvaluationScope(EvaluationScopePtr()) {}
		
	virtual VariableLookupPtr lookSymbol(Oop symbol);
};

VariableLookupPtr GlobalEvaluationScope::lookSymbol(Oop symbol)
{
	auto globalVar = getGlobalFromSymbol(symbol);
	if(globalVar.isNil())
		return VariableLookupPtr();

	return std::make_shared<LiteralVariableLookup> (globalVar);
}

// Scoped interpreter
class ScopedInterpreter: public ASTVisitor
{
public:
	ScopedInterpreter(const EvaluationScopePtr &initialScope = EvaluationScopePtr())
		: currentScope(initialScope) {}
	~ScopedInterpreter() {}
	
	void pushScope(EvaluationScopePtr newScope)
	{
		currentScope = newScope;
	}
	
	void popScope()
	{
		currentScope = currentScope->getParent();
	}
	
	void error(Node *location, const char *format, ...);
	
protected:
	EvaluationScopePtr currentScope;
};

void ScopedInterpreter::error(Node *location, const char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 1024, format, args);
	
	fputs(buffer, stderr);
	fputs("\n", stderr);
	abort();
}

// ASTInterpreter visitor
class ASTInterpreter: public ScopedInterpreter
{
public:
	ASTInterpreter(const EvaluationScopePtr &initialScope, Oop currentSelf)
		: ScopedInterpreter(initialScope), currentSelf(currentSelf) {}

	virtual Oop visitArgument(Argument *node);
	virtual Oop visitArgumentList(ArgumentList *node);
	virtual Oop visitAssignmentExpression(AssignmentExpression *node);
	virtual Oop visitBlockExpression(BlockExpression *node);
	virtual Oop visitIdentifierExpression(IdentifierExpression *node);
	virtual Oop visitLiteralNode(LiteralNode *node);
	virtual Oop visitLocalDeclarations(LocalDeclarations *node);
	virtual Oop visitLocalDeclaration(LocalDeclaration *node);
	virtual Oop visitMessageSendNode(MessageSendNode *node);
	virtual Oop visitMethodAST(MethodAST *node);
	virtual Oop visitMethodHeader(MethodHeader *node);
	virtual Oop visitReturnStatement(ReturnStatement *node);
	virtual Oop visitSequenceNode(SequenceNode *node);
	virtual Oop visitSelfReference(SelfReference *node);
	virtual Oop visitSuperReference(SuperReference *node);
	virtual Oop visitThisContextReference(ThisContextReference *node);
	
private:
	Oop currentSelf;
};

Oop ASTInterpreter::visitArgument(Argument *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop ASTInterpreter::visitArgumentList(ArgumentList *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop ASTInterpreter::visitAssignmentExpression(AssignmentExpression *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop ASTInterpreter::visitBlockExpression(BlockExpression *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop ASTInterpreter::visitIdentifierExpression(IdentifierExpression *node)
{
	VariableLookupPtr variable;
	
	// Find in the current scope.
	if(currentScope)
		variable = currentScope->lookSymbolRecursively(node->getSymbol());
		
	// Ensure it was found.
	if(!variable)
		error(node, "undeclared identifier '%s'.", node->getIdentifier().c_str());

	// Read the variable.
	return variable->getValue();
}

Oop ASTInterpreter::visitLiteralNode(LiteralNode *node)
{
	return node->getValue().getOop();
}

Oop ASTInterpreter::visitLocalDeclarations(LocalDeclarations *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop ASTInterpreter::visitLocalDeclaration(LocalDeclaration *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop ASTInterpreter::visitMessageSendNode(MessageSendNode *node)
{
	Oop result;
	std::vector<Oop> argumentValues;
	
	// Evaluate the receiver.
	auto receiver = node->getReceiver()->acceptVisitor(this);
	auto &chained = node->getChainedMessages();
	
	// Send each message in the chain
	for(int i = -1; i < (int)chained.size(); ++i)
	{
		auto message = i < 0 ? node : chained[i];
		auto selector = message->getSelectorOop();
		
		// Evaluate the arguments.
		auto &arguments = message->getArguments();
		argumentValues.clear();
		for(auto &arg : arguments)
			argumentValues.push_back(arg->acceptVisitor(this));

		// Send the message.
		result = sendMessage(receiver, selector, argumentValues.size(), &argumentValues[0]);
	}
	
	// Return the result.
	return result;
}

Oop ASTInterpreter::visitMethodAST(MethodAST *node)
{
	auto handle = reinterpret_cast<MethodASTHandle*> (MethodASTHandle::ClassObject->basicNativeNew());
	if(!isNil(handle))
		handle->ast = node;
	return Oop::fromPointer(handle);
}

Oop ASTInterpreter::visitMethodHeader(MethodHeader *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop ASTInterpreter::visitReturnStatement(ReturnStatement *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop ASTInterpreter::visitSequenceNode(SequenceNode *node)
{
	Oop result;
	if(node->getLocalDeclarations())
	{
		// TODO: Create the new scope
		assert(0 && "unimplemented");
		abort();
	}
	
	for(auto &child : node->getChildren())
		result = child->acceptVisitor(this);
	return result;
}

Oop ASTInterpreter::visitSelfReference(SelfReference *node)
{
	return currentSelf;
}

Oop ASTInterpreter::visitSuperReference(SuperReference *node)
{
	return currentSelf;
}

Oop ASTInterpreter::visitThisContextReference(ThisContextReference *node)
{
	error(node, "this context is not supported");
	abort();
}

// Method compiler
class MethodCompiler: public ScopedInterpreter
{
public:
	MethodCompiler(const EvaluationScopePtr &initialScope)
		: ScopedInterpreter(initialScope) {}

	virtual Oop visitArgument(Argument *node);
	virtual Oop visitArgumentList(ArgumentList *node);
	virtual Oop visitAssignmentExpression(AssignmentExpression *node);
	virtual Oop visitBlockExpression(BlockExpression *node);
	virtual Oop visitIdentifierExpression(IdentifierExpression *node);
	virtual Oop visitLiteralNode(LiteralNode *node);
	virtual Oop visitLocalDeclarations(LocalDeclarations *node);
	virtual Oop visitLocalDeclaration(LocalDeclaration *node);
	virtual Oop visitMessageSendNode(MessageSendNode *node);
	virtual Oop visitMethodAST(MethodAST *node);
	virtual Oop visitMethodHeader(MethodHeader *node);
	virtual Oop visitReturnStatement(ReturnStatement *node);
	virtual Oop visitSequenceNode(SequenceNode *node);
	virtual Oop visitSelfReference(SelfReference *node);
	virtual Oop visitSuperReference(SuperReference *node);
	virtual Oop visitThisContextReference(ThisContextReference *node);
};

// Method compiler.
Oop MethodCompiler::visitArgument(Argument *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop MethodCompiler::visitArgumentList(ArgumentList *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop MethodCompiler::visitAssignmentExpression(AssignmentExpression *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop MethodCompiler::visitBlockExpression(BlockExpression *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop MethodCompiler::visitIdentifierExpression(IdentifierExpression *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop MethodCompiler::visitLiteralNode(LiteralNode *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop MethodCompiler::visitLocalDeclarations(LocalDeclarations *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop MethodCompiler::visitLocalDeclaration(LocalDeclaration *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop MethodCompiler::visitMessageSendNode(MessageSendNode *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop MethodCompiler::visitMethodAST(MethodAST *node)
{
	// Visit the method header.
	node->getHeader()->acceptVisitor(this);
	
	// Visit the method body
	node->getBody()->acceptVisitor(this);
	
	return nilOop();
}

Oop MethodCompiler::visitMethodHeader(MethodHeader *node)
{
	return Oop();
}

Oop MethodCompiler::visitReturnStatement(ReturnStatement *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop MethodCompiler::visitSequenceNode(SequenceNode *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop MethodCompiler::visitSelfReference(SelfReference *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop MethodCompiler::visitSuperReference(SuperReference *node)
{
	assert(0 && "unimplemented");
	abort();
}

Oop MethodCompiler::visitThisContextReference(ThisContextReference *node)
{
	assert(0 && "unimplemented");
	abort();
}

// Compiler interface
Oop executeDoIt(const std::string &code)
{
	// TODO: implement this
	abort();
}

Oop executeScript(const std::string &code, const std::string name)
{
	// TODO: implement this
	abort();
}

Oop executeScriptFromFile(FILE *file, const std::string name)
{
	// Create the script context
	Ref<ScriptContext> context(reinterpret_cast<ScriptContext*> (ScriptContext::ClassObject->basicNativeNew()));
	if(context.isNil())
		return Oop();
		
	// Parse the script.
	auto ast = Lodtalk::AST::parseSourceFromFile(stdin);
	
	// Create the global scope
	auto scope = std::make_shared<GlobalEvaluationScope> ();
	
	// Interpret the script.
	ASTInterpreter interpreter(scope, context.getOop());
	auto result = ast->acceptVisitor(&interpreter);
	return result;
}


// ScriptContext
Oop ScriptContext::setCurrentCategory(Oop category)
{
	currentCategory = category;
	return selfOop();
}

Oop ScriptContext::setCurrentClass(Oop classObject)
{
	currentClass = classObject;
	return selfOop();
}

Oop ScriptContext::addFunction(Oop methodAstHandle)
{
	if(isNil(methodAstHandle))
		error("cannot add method with nil ast.");
	if(classIndexOf(methodAstHandle) != SCI_MethodASTHandle)
		error("expected a method AST handle.");

	// Get the ast
	MethodASTHandle *handle = reinterpret_cast<MethodASTHandle*> (methodAstHandle.pointer);
	auto ast = handle->ast;
	
	// Create the global scope
	auto globalScope = std::make_shared<GlobalEvaluationScope> ();
	
	// Compile the method
	MethodCompiler compiler(globalScope);
	auto compiledMethod = ast->acceptVisitor(&compiler);

	// Register the method

	// Return self
	return selfOop();
}

Oop ScriptContext::addMethod(Oop methodAstHandle)
{
	return selfOop();
}

// The script context
LODTALK_BEGIN_CLASS_SIDE_TABLE(ScriptContext)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(ScriptContext)
	LODTALK_METHOD("category:", &ScriptContext::setCurrentCategory)
	LODTALK_METHOD("class:", &ScriptContext::setCurrentClass)
	LODTALK_METHOD("function:", &ScriptContext::addFunction)
	LODTALK_METHOD("method:", &ScriptContext::addMethod)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(ScriptContext, Object, OF_FIXED_SIZE, 0);

// The method ast handle
LODTALK_BEGIN_CLASS_SIDE_TABLE(MethodASTHandle)
LODTALK_END_CLASS_SIDE_TABLE()

LODTALK_BEGIN_CLASS_TABLE(MethodASTHandle)
LODTALK_END_CLASS_TABLE()

LODTALK_SPECIAL_SUBCLASS_DEFINITION(MethodASTHandle, Object, OF_INDEXABLE_8, 0);

} // End of namespace Lodtalk
