#ifndef LODEN_PRINTING_HPP_
#define LODEN_PRINTING_HPP_

#include "Loden/Common.hpp"

namespace Loden
{
	
LODEN_CORE_EXPORT void printMessage(const char *format, ...);
LODEN_CORE_EXPORT void printError(const char *format, ...);
LODEN_CORE_EXPORT void printWarning(const char *format, ...);

} // End of namespace Loden

#endif //LODEN_PRINTING_HPP_
