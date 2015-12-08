#ifndef LODEN_JSON_HPP
#define LODEN_JSON_HPP

#include "Loden/Common.hpp"
#include "rapidjson/document.h"
#include <string>

namespace Loden
{

/**
 * Parses a Json from a file.
 */
LODEN_CORE_EXPORT bool parseJsonFromFile(const std::string &filename, rapidjson::Document *document);

} // End of namespace Loden

#endif //LODEN_JSON_HPP