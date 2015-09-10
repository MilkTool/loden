#include "Compiler.hpp"

using namespace Lodtalk;

int main(int argc, const char *argv[])
{
    //auto ioClass = getGlobalValueFromName("OSIO");
    //auto stdoutOop = sendMessageOopArgs(ioClass, makeSelector("stdout"));
    //auto dataOop = makeByteString("Hello World\n");
    //sendMessageOopArgs(ioClass, makeSelector("write:offset:size:to:"), dataOop, Oop::encodeSmallInteger(0), Oop::encodeSmallInteger(13), stdoutOop);
    
    executeScriptFromFile(stdin, "stdin");
    
    //auto ast = Lodtalk::AST::parseSourceFromFile(stdin);
    //Lodtalk::sendBasicNewWithSize(Lodtalk::Object::ClassObject->selfOop(), 4);
    return 0;
}

