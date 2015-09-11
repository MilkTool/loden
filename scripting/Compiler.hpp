#ifndef LODTALK_COMPILER_HPP
#define LODTALK_COMPILER_HPP

#include "AST.hpp"

namespace Lodtalk
{
// Script class
class ScriptContext: public Object
{
	ScriptContext() {}
	LODTALK_NATIVE_CLASS();
public:
	Oop setCurrentCategory(Oop category);
	Oop setCurrentClass(Oop classObject);
	Oop addFunction(Oop functionAst);
	Oop addMethod(Oop methodAst);

	Oop currentCategory;
	Oop currentClass;
	Oop globalContextClass;

};

// Method AST handle
class MethodASTHandle: public Object
{
	MethodASTHandle() {}
	LODTALK_NATIVE_CLASS();
public:

	AST::MethodAST *ast;
};


// Compiler interface
Oop executeDoIt(const std::string &code);
Oop executeScript(const std::string &code, const std::string name = "unnamed");
Oop executeScriptFromFile(FILE *file, const std::string name = "unnamed");

}

#endif //LODTALK_COMPILER_HPP
