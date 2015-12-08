#include "Loden/FileSystem.hpp"
#include "Loden/Printing.hpp"
#include <algorithm>
#include <vector>

namespace Loden
{

LODEN_CORE_EXPORT std::string readWholeFile(const std::string &fileName)
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


LODEN_CORE_EXPORT bool isAbsolutePath(const std::string &path)
{
#ifdef _WIN32
    return path.size() >= 2 && path[1] == ':';
#else
    return !path.empty() && path[0] == '/';
#endif
}

LODEN_CORE_EXPORT std::string dirname(const std::string &path)
{
    size_t endDir = path.rfind('/');
#ifdef _WIN32
    size_t endDirWin32 = path.rfind('\\');
    if (endDir == std::string::npos)
        endDir = endDirWin32;
    else if (endDirWin32 != std::string::npos)
        endDir = std::max(endDir, endDirWin32);
#endif

    if (endDir == std::string::npos)
        return std::string();
    return path.substr(0, endDir + 1);

}

LODEN_CORE_EXPORT std::string basename(const std::string &path)
{
    size_t endDir = path.rfind('/');
#ifdef _WIN32
    size_t endDirWin32 = path.rfind('\\');
    if (endDir == std::string::npos)
        endDir = endDirWin32;
    else if (endDirWin32 != std::string::npos)
        endDir = std::max(endDir, endDirWin32);
#endif

    if (endDir == std::string::npos)
        return path;
    return path.substr(endDir + 1);
}

LODEN_CORE_EXPORT std::string extensionOfPath(const std::string &path)
{
    auto pos = path.rfind('.');
    if (pos == std::string::npos)
        return std::string();
    return path.substr(pos);
}

LODEN_CORE_EXPORT std::string joinPath(const std::string &path1, const std::string &path2)
{
    if (path1.empty())
        return path2;
    if (isAbsolutePath(path2))
        return path2;
    if(path1.back() == '/' || path1.back() == '\\')
        return path1 + path2;
    return path1 + "/" + path2;
}

} // End of namespace Loden
