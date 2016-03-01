#include "Loden/Printing.hpp"

#include <stdarg.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif

namespace Loden
{

#ifdef _WIN32
inline bool isConsoleApplication()
{
    return GetConsoleCP() != 0;
}
#endif

LODEN_CORE_EXPORT void printMessage(const char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 1024, format, args);
#ifdef _WIN32
    if(isConsoleApplication())
        fputs(buffer, stdout);
    else
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
    if (isConsoleApplication())
        fputs(buffer, stderr);
    else
        OutputDebugStringA(buffer);
#else
    fputs(buffer, stderr);
#endif
    va_end(args);
}

LODEN_CORE_EXPORT void printWarning(const char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 1024, format, args);
#ifdef _WIN32
    if (isConsoleApplication())
        fputs(buffer, stderr);
    else
        OutputDebugStringA(buffer);
#else
    fputs(buffer, stderr);
#endif
    va_end(args);
}

} // End of namespace Loden
