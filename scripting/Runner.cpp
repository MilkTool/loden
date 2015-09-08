#include "ParserScannerInterface.hpp"

int main(int argc, const char *argv[])
{
    auto ast = Lodtalk::AST::parseSourceFromFile(stdin);
    Lodtalk::sendBasicNewWithSize(Lodtalk::Object::ClassObject->selfOop(), 4);
    return 0;
}

