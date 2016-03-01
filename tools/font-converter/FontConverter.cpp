#include "Loden/Common.hpp"
#include "Loden/Math.hpp"
#include "Loden/Image/ImageBuffer.hpp"
#include "Loden/Image/Drawing.hpp"
#include "Loden/Image/Downsample.hpp"
#include "Loden/Image/SignedDistanceFieldTransform.hpp"

#include <string>
#include <string.h>
#include <stdio.h>
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace Loden;
using namespace Loden::Image;

static std::string inputFileName;
static std::string outputName;
static int cellWidth = 16;
static int cellHeight = 16;
static int sampleScale = 16;
static int cellMargin = 1;
static int sampleWidth;
static int sampleHeight;
static int faceWidth;
static int faceHeight;
static int columns = 128;
static int rows;
static int outputWidth;
static int outputHeight;

static FT_Library ftLibrary;
static FT_Face  face;

std::unique_ptr<LocalImageBuffer> sampleBuffer;
std::unique_ptr<LocalImageBuffer> distanceTransformBuffer;
std::unique_ptr<DoubleImageBuffer> downsampleBuffer;
std::unique_ptr<LocalImageBuffer> resultBuffer;

void printHelp()
{
}

void dumpBuffer(ImageBuffer *buffer)
{
    auto f = fopen("dump.data", "wb");
    fwrite(buffer->get(), buffer->getSize(), 1, f);
    fclose(f);
}

void convertGlyph(int glyphIndex, int resultRow, int resultColumn)
{
    clearImageBuffer(sampleBuffer.get());

    auto error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
    if (error)
    {
        fprintf(stderr, "Failed to load the glyph %d.\n", glyphIndex);
        return;
    }

    error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
    if (error)
    {
        fprintf(stderr, "Failed to render the glyph %d.\n", glyphIndex);
        return;
    }

    auto &bitmap = face->glyph->bitmap;
    auto width = bitmap.width;
    auto height = bitmap.rows;
    if (width > sampleWidth || height > sampleHeight)
    {
        fprintf(stderr, "Waring: Failed to convert glyph %d.\n", glyphIndex);
        return;
    }

    auto destX = (sampleWidth - width) / 2;
    auto destY = (sampleHeight - height) / 2;

    // Convert the bitmap into single byte image.
    ExternalImageBuffer bitmapBuffer(bitmap.width, bitmap.rows, bitmap.pitch, bitmap.buffer);
    expandBitmap<PixelR8> (destX, destY, sampleBuffer.get(), &bitmapBuffer);

    // Downsample
    downsample<PixelR8> (downsampleBuffer.get(), sampleBuffer.get(), sampleScale);

    // Copy to the result
    int resultX = resultColumn * cellWidth;
    int resultY = resultRow * cellHeight;
    copyRectangle<PixelR8> (resultX, resultY, resultBuffer.get(), 0, 0, cellWidth, cellHeight, downsampleBuffer.get());
}

int main(int argc, const char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-o"))
        {
            outputName = argv[++i];
        }
        else if (!strcmp(argv[i], "-cw"))
        {
            cellWidth = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-ch"))
        {
            cellHeight = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-help"))
        {
            printHelp();
            return 0;
        }
        else if(argv[i][0] != '-')
        {
            inputFileName = argv[i];
        }
    }

    if (inputFileName.empty() || outputName.empty())
    {
        printHelp();
        return -1;
    }

    sampleWidth = cellWidth * sampleScale;
    sampleHeight = cellHeight * sampleScale;
    faceWidth = sampleWidth - cellMargin * sampleScale*2;
    faceHeight= sampleHeight - cellMargin * sampleScale*2;

    auto error = FT_Init_FreeType(&ftLibrary);
    if (error)
    {
        fprintf(stderr, "Failed to initialize freetype.\n");
        return -1;
    }

    error = FT_New_Face(ftLibrary, inputFileName.c_str(), 0, &face);
    if (error == FT_Err_Unknown_File_Format)
    {
        fprintf(stderr, "Unsupported font format.\n");
        return -1;
    }
    else if (error)
    {
        fprintf(stderr, "Failed to load the font.\n");
        return -1;
    }

    error = FT_Set_Pixel_Sizes(face, faceWidth, faceHeight);
    if (error)
    {
        fprintf(stderr, "Failed to set the font face size.\n");
        return -1;
    }

    // Get the number of glyphs.
    int numberOfGlyphs = face->num_glyphs;
    printf("Number of avaialable glyphs: %d\n", numberOfGlyphs);

    // Compute the number of rows.
    rows = (numberOfGlyphs + columns - 1) / columns;
    rows = sameOrNextPowerOfTwo(rows);

    outputWidth = columns*cellWidth;
    outputHeight = rows*cellHeight;
    printf("Ouput grid size: %dx%d\n", columns, rows);
    printf("Ouput bitmap size: %dx%d\n", outputWidth, outputHeight);

    sampleBuffer.reset(new LocalImageBuffer(sampleWidth, sampleHeight, sampleWidth));
    distanceTransformBuffer.reset(new LocalImageBuffer(sampleWidth, sampleHeight, sampleWidth));
    downsampleBuffer.reset(new DoubleImageBuffer(sampleWidth, sampleHeight, sampleWidth));
    resultBuffer.reset(new LocalImageBuffer(outputWidth, outputHeight, outputWidth));

    // Clear the result buffer.
    clearImageBuffer(resultBuffer.get());

    // Convert the glyphs.
    for (int i = 0; i < face->num_glyphs; ++i)
    {
        convertGlyph(i, i / columns, i % columns);
    }

    dumpBuffer(resultBuffer.get());

    FT_Done_Face(face);
    FT_Done_FreeType(ftLibrary);

    return 0;
}
