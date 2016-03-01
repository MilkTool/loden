#include "Loden/Image/ReadWrite.hpp"
#include "Loden/Stdio.hpp"
#include "Loden/Printing.hpp"
#include <png.h>
#include <stdio.h>

namespace Loden
{
namespace Image
{

static void pngErrorFunction(png_structp s, const char *message)
{
    printError("libpng: %s\n", message);
}

static void pngWarningFunction(png_structp s, const char *message)
{
    printWarning("libpng: %s\n", message);
}

inline int getPngDepthForBpp(int bpp)
{
    return 8;
}

inline int getPngColorTypeForBpp(int bpp)
{
    switch (bpp)
    {
    case 8: return PNG_COLOR_TYPE_GRAY;
    case 16: return PNG_COLOR_TYPE_GRAY_ALPHA;
    case 32: return PNG_COLOR_TYPE_RGBA;
    default: abort();
    }
}

bool saveImageAsPng(const std::string &fileName, ImageBuffer *imageBuffer)
{
    OutputStdFile out;
    if (!out.open(fileName, true))
        return false;

    auto pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, pngErrorFunction, pngWarningFunction);
    if (!pngPtr)
        return false;

    auto infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr)
    {
        png_destroy_write_struct(&pngPtr, nullptr);
        return false;
    }
    
    if (setjmp(png_jmpbuf(pngPtr)))
    {
        png_destroy_write_struct(&pngPtr, &infoPtr);
        return false;
    }

    png_init_io(pngPtr, out.get());

    // Write the image header.
    auto bufferBpp = imageBuffer->getBitsPerPixel();
    png_set_IHDR(pngPtr, infoPtr,
        imageBuffer->getWidth(), imageBuffer->getHeight(),
        getPngDepthForBpp(bufferBpp), getPngColorTypeForBpp(bufferBpp),
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    // Set the rows pointer
    std::unique_ptr<uint8_t*[]> rowPointers(new uint8_t *[imageBuffer->getHeight()]);
    auto row = imageBuffer->get();
    auto imagePitch = imageBuffer->getPitch();
    auto imageHeight = imageBuffer->getHeight();
    for (size_t y = 0; y < imageHeight; ++y, row += imagePitch)
        rowPointers[y] = row;

    png_set_rows(pngPtr, infoPtr, rowPointers.get());

    // Write the png data.
    png_write_png(pngPtr, infoPtr, PNG_TRANSFORM_IDENTITY, nullptr);

    // Finish
    png_write_end(pngPtr, infoPtr);

    png_destroy_write_struct(&pngPtr, &infoPtr);
    out.commit();
    return true;
}
} // End of namespace Image
} // End of namespace Loden
