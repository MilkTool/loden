#ifndef LODEN_FILESYSTEM_HPP
#define LODEN_FILESYSTEM_HPP

#include "Loden/Common.hpp"
#include <string>

namespace Loden
{

/**
 * Tells if a paths is absolute.
 */
LODEN_CORE_EXPORT bool isAbsolutePath(const std::string &path);

/**
 * Extracts the dirname portion of the path
 */
LODEN_CORE_EXPORT std::string dirname(const std::string &path);

/**
* Extracts the base name portion of the path.
*/
LODEN_CORE_EXPORT std::string basename(const std::string &path);

/**
* Extracts the extension portion of the path.
*/
LODEN_CORE_EXPORT std::string extensionOfPath(const std::string &path);

/**
* Joins a base path with a relative path.
*/
LODEN_CORE_EXPORT std::string joinPath(const std::string &path1, const std::string &path2);

} // End of namespace

#endif //LODEN_FILESYSTEM_HPP