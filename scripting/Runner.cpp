#include <string>
#include <stdio.h>
#include <string.h>
#include "Compiler.hpp"

using namespace Lodtalk;

void printHelp()
{
}

int main(int argc, const char *argv[])
{
    std::string scriptFilename;
    
    for(int i = 1; i < argc; ++i)
    {
        if(!strcmp(argv[i], "-help") ||
           !strcmp(argv[i], "-h"))
        {
            printHelp();
            return 0;
        }
        else
        {
            scriptFilename = argv[i];
        }
    }
    
    if(scriptFilename.empty())
    {
        printHelp();
        return -1;
    }
    
    if(scriptFilename == "-")
    {
        // Execute script from the standard input.
        executeScriptFromFile(stdin, "stdin");
    }
    else
    {
        auto file = fopen(scriptFilename.c_str(), "r");
        if(!file)
        {
            fprintf(stderr, "Failed to open file: %s\n", scriptFilename.c_str());
            return -1;
        }
        
        // Execute the script from the file.
        executeScriptFromFile(file, scriptFilename);
        fclose(file);
    }
    
    // Call the main function.
    auto globalContext = getGlobalContext();
    sendMessageOopArgs(globalContext, makeSelector("main"));
    return 0;
}
