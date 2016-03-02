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

inline int getChannelCountForColorType(int colorType)
{
    switch (colorType)
    {
    case PNG_COLOR_TYPE_GRAY: return 1;
    case PNG_COLOR_TYPE_GRAY_ALPHA: return 2;
    case PNG_COLOR_TYPE_RGB: return 4;
    case PNG_COLOR_TYPE_RGBA: return 4;
    case PNG_COLOR_TYPE_PALETTE: return 4;
    default: return -1;
    }
}
static int readChunkCallback(png_structp ptr, png_unknown_chunkp chunk)
{
    return 0;
}

ImageBufferPtr loadImageFromPng(const std::string &fileName)
{
    std::unique_ptr<uint8_t *[]> rowPointers;
    InputStdFile in;
    if (!in.open(fileName, true))
        return nullptr;

    // Check the PNG signature.
    uint8_t header[8];
    if (fread(header, 8, 1, in.get()) != 1)
        return nullptr;

    auto isPng = !png_sig_cmp(header, 0, 8);
    if (!isPng)
        return nullptr;

    // Allocate read structures.
    auto pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, pngErrorFunction, pngWarningFunction);
    if (!pngPtr)
        return nullptr;

    auto infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr)
    {
        png_destroy_read_struct(&pngPtr, nullptr, nullptr);
        return nullptr;
    }

    auto endInfoPtr = png_create_info_struct(pngPtr);
    if (!endInfoPtr)
    {
        png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
        return nullptr;
    }

    // Handle read errors
    if (setjmp(png_jmpbuf(pngPtr)))
    {
        png_destroy_read_struct(&pngPtr, &infoPtr, &endInfoPtr);
        return nullptr;
    }

    png_init_io(pngPtr, in.get());
    png_set_sig_bytes(pngPtr, 8);

    // Set the unknown chunk callback.
    png_set_read_user_chunk_fn(pngPtr, nullptr, readChunkCallback);

    // Read the info ptr.
    png_read_info(pngPtr, infoPtr);

    uint32_t width;
    uint32_t height;
    int bitDepth;
    int colorType;
    int interlaceMethod;
    int compressionMethod;
    int filterMethod;
    png_get_IHDR(pngPtr, infoPtr, &width, &height, &bitDepth, &colorType, &interlaceMethod, &compressionMethod, &filterMethod);

    // Set some transformations
    if (colorType == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(pngPtr);
    if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
        png_set_expand_gray_1_2_4_to_8(pngPtr);
    if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(pngPtr);

    if (bitDepth < 8)
        png_set_packing(pngPtr);

    if (colorType == PNG_COLOR_TYPE_RGB)
        png_set_filler(pngPtr, ~0u, PNG_FILLER_BEFORE);
    png_read_update_info(pngPtr, infoPtr);
    png_get_IHDR(pngPtr, infoPtr, &width, &height, &bitDepth, &colorType, &interlaceMethod, &compressionMethod, &filterMethod);

    auto channelCount = getChannelCountForColorType(colorType);
    if (channelCount < 0)
    {
        printError("Trying to load png image with unsupported color type\n");
        png_destroy_read_struct(&pngPtr, &infoPtr, &endInfoPtr);
        return nullptr;
    }

    // Allocate an image for the result
    auto bpp = channelCount*bitDepth;
    auto pitch = ((width *bpp) / 8 + 3) & (~3);
    auto loadedImage = std::make_shared <LocalImageBuffer>(width, height, bpp, pitch);

    // Set the row pointers.
    rowPointers.reset(new uint8_t *[height]);
    auto row = loadedImage->get();
    for (size_t y = 0; y < height; ++y, row += pitch)
        rowPointers[y] = row;

    png_read_image(pngPtr, rowPointers.get());
    
    png_read_end(pngPtr, infoPtr);
    png_destroy_read_struct(&pngPtr, &infoPtr, &endInfoPtr);
    return loadedImage;
}

bool saveImageAsPng(const std::string &fileName, ImageBuffer *imageBuffer)
{
    std::unique_ptr<uint8_t*[]> rowPointers;
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
    rowPointers.reset(new uint8_t *[imageBuffer->getHeight()]);
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
