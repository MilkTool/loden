#include "AST.hpp"

namespace Lodtalk
{
namespace AST
{
Node::Node()
{
}

Node::~Node()
{
}

// Identifier expression
IdentifierExpression::IdentifierExpression(const std::string &identifier)
	: identifier(identifier)
{
}

IdentifierExpression::~IdentifierExpression()
{
}

const std::string &IdentifierExpression::getIdentifier() const
{
	return identifier;
}

// Literal node
LiteralNode::LiteralNode(const Ref<ProtoObject> &value)
	: value(value)
{
}

LiteralNode::~LiteralNode()
{
}

const Ref<ProtoObject> &LiteralNode::getValue() const
{
	return value;
}

// Message send
MessageSendNode::MessageSendNode(const std::string &selector, Node *receiver)
	: selector(selector), receiver(receiver)
{
}

MessageSendNode::MessageSendNode(const std::string &selector, Node *receiver, Node *firstArgument)
	: selector(selector), receiver(receiver)
{
	arguments.push_back(firstArgument);
}

MessageSendNode::~MessageSendNode()
{
	delete receiver;
	for(auto &arg : arguments)
		delete arg;
}

const std::string &MessageSendNode::getSelector() const
{
	return selector;
}

Node *MessageSendNode::getReceiver() const
{
	return receiver;
}

void MessageSendNode::setReceiver(Node *newReceiver)
{
	receiver = newReceiver;
}

const std::vector<Node*> &MessageSendNode::getArguments() const
{
	return arguments;
}

void MessageSendNode::appendSelector(const std::string &selectorExtra)
{
	selector += selectorExtra;
}

void MessageSendNode::appendArgument(Node *newArgument)
{
	arguments.push_back(newArgument);
}

void MessageSendNode::appendChained(MessageSendNode *chainedMessage)
{
	chainedMessages.push_back(chainedMessage);
}

// Assignment expression
AssignmentExpression::AssignmentExpression(Node *reference, Node *value)
	: reference(reference), value(value)
{
}

AssignmentExpression::~AssignmentExpression()
{
	delete reference;
	delete value;
}

Node *AssignmentExpression::getReference() const
{
	return reference;
}

Node *AssignmentExpression::getValue() const
{
	return value;
}

// Sequence node	
SequenceNode::SequenceNode(Node *first)
{
	if(first)
		children.push_back(first);
}

SequenceNode::~SequenceNode()
{
	for(auto &child : children)
		delete child;
}

void SequenceNode::addStatement(Node *node)
{
	children.push_back(node);
}

LocalDeclarations *SequenceNode::getLocalDeclarations() const
{
	return localDeclarations;
}

void SequenceNode::setLocalDeclarations(LocalDeclarations *newLocals)
{
	localDeclarations = newLocals;
}

// Return statement
ReturnStatement::ReturnStatement(Node *value)
	: value(value)
{
}

ReturnStatement::~ReturnStatement()
{
	delete value;
}

Node *ReturnStatement::getValue() const
{
	return value;
}

// Local declaration
LocalDeclaration::LocalDeclaration(const std::string &name)
	: name(name)
{
}

LocalDeclaration::~LocalDeclaration()
{
}

const std::string &LocalDeclaration::getName() const
{
	return name;
}

// Local declarations
LocalDeclarations::LocalDeclarations()
{
}

LocalDeclarations::~LocalDeclarations()
{
}

const std::vector<LocalDeclaration*> &LocalDeclarations::getLocals() const
{
	return locals;
}

void LocalDeclarations::appendLocal(LocalDeclaration *local)
{
	locals.push_back(local);
}

// Argument
Argument::Argument(const std::string &name)
	: name(name)
{
}

Argument::~Argument()
{
}

const std::string &Argument::getName()
{
	return name;
}

// Argument list
ArgumentList::ArgumentList(Argument *firstArgument)
{
	arguments.push_back(firstArgument);
}

ArgumentList::~ArgumentList()
{
	for(auto &child : arguments)
		delete child;
}

const std::vector<Argument*> &ArgumentList::getArguments()
{
	return arguments;
}

void ArgumentList::appendArgument(Argument *argument)
{
	arguments.push_back(argument);	
}

// Block expression
BlockExpression::BlockExpression(ArgumentList *argumentList, SequenceNode *body)
	: argumentList(argumentList), body(body)
{
}

BlockExpression::~BlockExpression()
{
	delete argumentList;
	delete body;
}

ArgumentList *BlockExpression::getArgumentList() const
{
	return argumentList;
}

SequenceNode *BlockExpression::getBody() const
{
	return body;
}

// Method header
MethodHeader::MethodHeader(const std::string &selector, ArgumentList *arguments)
	: selector(selector), arguments(arguments)
{
}

MethodHeader::~MethodHeader()
{
	delete arguments;
}

ArgumentList *MethodHeader::getArgumentList() const
{
	return arguments;
}

void MethodHeader::appendSelectorAndArgument(const std::string &selectorExtra, Argument *argument)
{
	selector += selectorExtra;
	arguments->appendArgument(argument);
}

// Method AST
MethodAST::MethodAST(MethodHeader *header, Node *pragmas, SequenceNode *body)
	: header(header), body(body)
{
}

MethodAST::~MethodAST()
{
	delete header;
	delete body;
}

MethodHeader *MethodAST::getHeader() const
{
	return header;
}

SequenceNode *MethodAST::getBody() const
{
	return body;
}

} // End of namespace AST	
} // End of namespace Lodtalk
