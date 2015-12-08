#include "Loden/JSON.hpp"
#include "Loden/FileSystem.hpp"
#include "Loden/Printing.hpp"
#include <vector>

namespace Loden
{

LODEN_CORE_EXPORT bool parseJsonFromFile(const std::string &filename, rapidjson::Document *document)
{
    auto data = readWholeFile(filename);
    if (data.empty())
        return false;

    document->Parse(data.c_str());
    if (document->HasParseError())
    {
        printError("JSON parsing error in (%d)'%s'\n", (int)document->GetErrorOffset(), filename.c_str());
        return false;
    }
    return true;
} 

} // End of namspace Loden
