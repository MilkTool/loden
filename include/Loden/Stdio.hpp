#ifndef LODEN_STDIO_HPP
#define LODEN_STDIO_HPP

#include <stdio.h>
#include <string>

namespace Loden
{

/**
* Input C standard library file, it can be commited.
*/
class InputStdFile
{
public:
    InputStdFile()
        : handle(nullptr)
    {
    }

    ~InputStdFile()
    {
        if (handle)
            close();
    }

    bool open(const std::string &fileName, bool binary = true)
    {
        assert(!handle);
        this->handle = fopen(fileName.c_str(), binary ? "rb" : "r");
        return handle != nullptr;
    }

    void close()
    {
        if(handle)
            fclose(handle);
        handle = nullptr;
    }

    FILE *get()
    {
        return handle;
    }

private:
    FILE *handle;
};

/**
 * Output C standard library file, it can be commited.
 */
class OutputStdFile
{
public:
    OutputStdFile()
        : handle(nullptr)
    {
    }

    ~OutputStdFile()
    {
        if (handle)
            abort();
    }

    bool open(const std::string &fileName, bool binary = true)
    {
        assert(!handle);
        this->fileName = fileName;
        this->handle = fopen(fileName.c_str(), binary ? "wb" : "w");
        return handle != nullptr;
    }

    void commit()
    {
        if (!handle)
            return;

        fclose(handle);
        handle = nullptr;
    }

    void abort()
    {
        if (!handle)
            return;

        fclose(handle);
        remove(fileName.c_str());
        handle = nullptr;
    }

    FILE *get()
    {
        return handle;
    }

private:
    std::string fileName;
    FILE *handle;
};
 
} // End of namespace Loden

#endif //LODEN_STDIO_HPP
