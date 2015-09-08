#include <stdarg.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "Loden/Printing.hpp"

namespace Loden
{
	
LODEN_CORE_EXPORT void printMessage(const char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 1024, format, args);
#ifdef _WIN32
    OutputDebugStringA(buffer);
#else
    fputs(buffer, stdout);
#endif
    va_end(args);
}

LODEN_CORE_EXPORT void printError(const char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 1024, format, args);
#ifdef _WIN32
    OutputDebugStringA(buffer);
#else
    fputs(buffer, stderr);
#endif
    va_end(args);
}

} // End of namespace Loden
