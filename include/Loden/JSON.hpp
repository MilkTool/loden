#ifndef LODEN_JSON_HPP
#define LODEN_JSON_HPP

#include "rapidjson/document.h"
#include <string>

namespace Loden
{

std::string readWholeFile(const std::string &fileName);
bool parseJsonFromFile(const std::string &filename, rapidjson::Document *document);

} // End of namespace Loden

#endif //LODEN_JSON_HPP