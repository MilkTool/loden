#include "Loden/JSON.hpp"
#include "Loden/Printing.hpp"
#include <vector>

namespace Loden
{

std::string readWholeFile(const std::string &fileName)
{
    FILE *file = fopen(fileName.c_str(), "rb");
    if (!file)
    {
        printError("Failed to open file %s\n", fileName.c_str());
        return std::string();
    }

    // Allocate the data.
    std::vector<char> data;
    fseek(file, 0, SEEK_END);
    data.resize(ftell(file));
    fseek(file, 0, SEEK_SET);

    // Read the file
    if (fread(&data[0], data.size(), 1, file) != 1)
    {
        printError("Failed to read file %s\n", fileName.c_str());
        fclose(file);
        return std::string();
    }

    fclose(file);
    return std::string(data.begin(), data.end());
}

bool parseJsonFromFile(const std::string &filename, rapidjson::Document *document)
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
