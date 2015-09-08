#ifndef LODTALK_AST_HPP_
#define LODTALK_AST_HPP_

#include <vector>
#include <string>
#include "Object.hpp"

namespace Lodtalk
{
namespace AST
{

class Node;
class LiteralNode;
class MessageSendNode;
class LocalDeclarations;
class LocalDeclaration;
class IdentifierExpression;
class AssignmentExpression;
class SequenceNode;
class ReturnStatement;

/**
 * AST visitor
 */
class ASTVisitor
{
	
};

/**
 * AST node
 */	
class Node
{
public:
	Node();
	~Node();
};

/**
 * Literal node
 */
class LiteralNode: public Node
{
public:
	LiteralNode(const Ref<ProtoObject> &value);
	~LiteralNode();
	
	const Ref<ProtoObject> &getValue() const;
	
private:
	Ref<ProtoObject> value;
};

/**
 * Message send
 */
class MessageSendNode: public Node
{
public:
	MessageSendNode(const std::string &selector, Node *receiver);
	MessageSendNode(const std::string &selector, Node *receiver, Node *firstArgument);
	~MessageSendNode();

	const std::string &getSelector() const;
	const std::vector<Node*> &getArguments() const;
	
	Node *getReceiver() const;
	void setReceiver(Node *newReceiver);
	
	void appendSelector(const std::string &selectorExtra);
	void appendArgument(Node *newArgument);
	void appendChained(MessageSendNode *chainedMessage);
	
private:
	std::string selector;
	Node *receiver;
	std::vector<Node*> arguments;
	std::vector<MessageSendNode*> chainedMessages;
};

/**
 * Identifier expression
 */
class IdentifierExpression: public Node
{
public:
	IdentifierExpression(const std::string &identifier);
	~IdentifierExpression();
	
	const std::string &getIdentifier() const;
	
private:
	std::string identifier;	
};

/**
 * Assignment expression node
 */
class AssignmentExpression: public Node
{
public:
	AssignmentExpression(Node *reference, Node *value);
	~AssignmentExpression();
	
	Node *getReference() const;
	Node *getValue() const;
	
private:
	Node *reference;
	Node *value;
};

/**
 * Sequence node
 */
class SequenceNode: public Node
{
public:
	SequenceNode(Node *first=nullptr);
	~SequenceNode();
	
	void addStatement(Node *node);
	
	LocalDeclarations *getLocalDeclarations() const;
	void setLocalDeclarations(LocalDeclarations *newLocals);
	
private:
	std::vector<Node*> children;
	LocalDeclarations *localDeclarations;
};

/**
 * Return statement
 */
class ReturnStatement: public Node
{
public:
	ReturnStatement(Node *value);
	~ReturnStatement();
	
	Node *getValue() const;
	
private:
	Node *value;
};

/**
 * Local variable declaration.
 */
class LocalDeclaration: public Node
{
public:
	LocalDeclaration(const std::string &name);
	~LocalDeclaration();
	
	const std::string &getName() const;
	
private:
	std::string name;
};

/**
 * Local declarations.
 */
class LocalDeclarations: public Node
{
public:
	LocalDeclarations();
	~LocalDeclarations();
	
	const std::vector<LocalDeclaration*> &getLocals() const;
	
	void appendLocal(LocalDeclaration *local);
	
private:
	std::vector<LocalDeclaration*> locals;
};

/**
 * Argument
 */
class Argument: public Node
{
public:
	Argument(const std::string &name);
	~Argument();
	
	const std::string &getName();
	
private:
	std::string name;
};

/**
 * Argument list
 */
class ArgumentList: public Node
{
public:
	ArgumentList(Argument *firstArgument);
	~ArgumentList();
	
	const std::vector<Argument*> &getArguments();
	
	void appendArgument(Argument *argument);
	
private:
	std::vector<Argument*> arguments;
};

/**
 * Block expression
 */
class BlockExpression: public Node
{
public:
	BlockExpression(ArgumentList *argumentList, SequenceNode *body);
	~BlockExpression();
	
	ArgumentList *getArgumentList() const;
	SequenceNode *getBody() const;
	
private:
	ArgumentList *argumentList;
	SequenceNode *body;
};

/**
 * Method header
 */
class MethodHeader: public Node
{
public:
	MethodHeader(const std::string &selector, ArgumentList *arguments = nullptr);
	~MethodHeader();
	
	ArgumentList *getArgumentList() const;

	void appendSelectorAndArgument(const std::string &selector, Argument *argument);
		
private:
	std::string selector;
	ArgumentList *arguments;
};

/**
 * Method AST
 */
class MethodAST: public Node
{
public:
	MethodAST(MethodHeader *header, Node *pragmas, SequenceNode *body);
	~MethodAST();

	MethodHeader *getHeader() const;
	SequenceNode *getBody() const;
	
private:
	MethodHeader *header;
	SequenceNode *body;
};

/**
 * Self reference
 */
class SelfReference: public Node
{
};

/**
 * Super reference
 */
class SuperReference: public Node
{
};

/**
 * ThisContext reference
 */
class ThisContextReference: public Node
{
};


} // End of namespace AST
} // End of namespace Lodtalk

#endif //LODTALK_AST_HPP_
