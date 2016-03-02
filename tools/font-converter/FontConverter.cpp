#include "Loden/Common.hpp"
#include "Loden/Math.hpp"
#include "Loden/Printing.hpp"
#include "Loden/Image/ImageBuffer.hpp"
#include "Loden/Image/Drawing.hpp"
#include "Loden/Image/Downsample.hpp"
#include "Loden/Image/SignedDistanceFieldTransform.hpp"
#include "Loden/Image/ReadWrite.hpp"

#include "Loden/GUI/LodenFontFormat.hpp"

#include "Loden/Stdio.hpp"

#include <thread>
#include <mutex>
#include <queue>

#include <string>
#include <string.h>
#include <stdio.h>
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace Loden;
using namespace Loden::GUI;
using namespace Loden::Image;

static std::string inputFileName;
static std::string outputName;
static int cellWidth = 16;
static int cellHeight = 16;
static int sampleScale = 4;
static int cellMargin = 1;
static int sampleWidth;
static int sampleHeight;
static int faceWidth;
static int faceHeight;
static int columns = 128;
static int rows;
static int outputWidth;
static int outputHeight;
static int numberOfJobs = 8;
static int failCount = 0;
static bool distanceFieldFont = false;
static std::vector<bool> glyphConvertionSuccess;

static FT_Library ftLibrary;
static FT_Face  face;

std::unique_ptr<LocalImageBuffer> resultBuffer;
std::vector<LodenFontCharMapEntry> characterMap;

void printHelp()
{
}

std::vector<LodenFontGlyphMetadata> glyphMetadata;

class GlyphConverter;
std::queue<GlyphConverter*> converterWaitingQueue;
std::mutex converterWaitingQueueMutex;
std::condition_variable converterWaitingQueueCondition;

void addConverterToQueue(GlyphConverter *converter)
{
    std::unique_lock<std::mutex> l(converterWaitingQueueMutex);
    converterWaitingQueue.push(converter);
    converterWaitingQueueCondition.notify_all();
}

class GlyphConverter
{
public:
    GlyphConverter()
    {
        hasConvertionJob = false;
        shuttingDown = false;

        sampleBuffer.reset(new LocalImageBuffer(sampleWidth, sampleHeight, 8, sampleWidth));
        distanceTransformBuffer.reset(new LocalImageBuffer(sampleWidth, sampleHeight, 8, sampleWidth));
        downsampleBuffer.reset(new DoubleImageBuffer(sampleWidth, sampleHeight, 8, sampleWidth));
    }

    ~GlyphConverter()
    {
    }

    void start()
    {
        shuttingDown = false;
        std::thread t([=] {
            converterThread();
        });

        thread.swap(t);
    }

    void shutdown()
    {
        {
            std::unique_lock<std::mutex> l(controlMutex);
            shuttingDown = true;
            hasJobCondition.notify_all();
        }
        thread.join();
    }

    template<typename FT>
    void beginConvertion(int glyphIndex, int resultRow, int resultColumn, const FT &function)
    {
        std::unique_lock<std::mutex> l(controlMutex);
        this->glyphIndex = glyphIndex;
        this->resultRow = resultRow;
        this->resultColumn = resultColumn;
        function(sampleBuffer.get());
        hasConvertionJob = true;
        hasJobCondition.notify_all();
    }

private:
    void convert();
    void converterThread();

    int glyphIndex;
    int resultRow;
    int resultColumn;
    bool hasConvertionJob;
    bool shuttingDown;

    std::unique_ptr<LocalImageBuffer> sampleBuffer;
    std::unique_ptr<LocalImageBuffer> distanceTransformBuffer;
    std::unique_ptr<DoubleImageBuffer> downsampleBuffer;

    std::mutex controlMutex;
    std::condition_variable hasJobCondition;
    std::thread thread;
};

void GlyphConverter::converterThread()
{
    for (;;)
    {
        hasConvertionJob = false;
        addConverterToQueue(this);

        {
            std::unique_lock<std::mutex> l(controlMutex);
            if (!hasConvertionJob || !shuttingDown)
                hasJobCondition.wait(l);

            if (shuttingDown && !hasConvertionJob)
                return;
        }

        convert();
        if (shuttingDown)
            return;
    }
}

void GlyphConverter::convert()
{
    if (distanceFieldFont)
    {
        // Compute the distance field map.
        clearImageBuffer(distanceTransformBuffer.get());
        computeSignedDistanceField<PixelR8s>(distanceTransformBuffer.get(), sampleBuffer.get(), (float)sampleScale);

        // Downsample
        downsample<PixelR8s>(downsampleBuffer.get(), distanceTransformBuffer.get(), sampleScale);

    }
    else
    {
        // Just downsample.
        downsample<PixelR8>(downsampleBuffer.get(), sampleBuffer.get(), sampleScale);
    }

    // Copy to the result
    int resultX = resultColumn * cellWidth;
    int resultY = resultRow * cellHeight;
    copyRectangle<PixelR8s> (resultX, resultY, resultBuffer.get(), 0, 0, cellWidth, cellHeight, downsampleBuffer.get());
}

template<typename FT>
void queueInConverter(int glyphIndex, int resultRow, int resultColumn, const FT &function)
{
    GlyphConverter *converter;
    {
        std::unique_lock<std::mutex> l(converterWaitingQueueMutex);

        while (converterWaitingQueue.empty())
            converterWaitingQueueCondition.wait(l);

        converter = converterWaitingQueue.front();
        converterWaitingQueue.pop();
    }

    converter->beginConvertion(glyphIndex, resultRow, resultColumn, function);
}

void startGlyphConvertion(int glyphIndex, int resultRow, int resultColumn)
{
    auto error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
    if (error)
    {
        ++failCount;
        printWarning("Failed to load the glyph %d.\n", glyphIndex);
        return;
    }

    error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
    if (error)
    {
        ++failCount;
        printWarning("Failed to render the glyph %d.\n", glyphIndex);
        return;
    }

    auto &bitmap = face->glyph->bitmap;
    int width = bitmap.width;
    int height = bitmap.rows;
    if (width > sampleWidth || height > sampleHeight)
    {
        ++failCount;
        printWarning("Failed to convert glyph %d. Size %dx%d bigger than cell size.\n", glyphIndex, width, height);
        return;
    }

    auto destX = (sampleWidth - width) / 2;
    auto destY = (sampleHeight - height) / 2;

    // Convert the bitmap into single byte image.
    queueInConverter(glyphIndex, resultRow, resultColumn, [&](ImageBuffer *sampleBuffer) {
        clearImageBuffer(sampleBuffer);
        ExternalImageBuffer bitmapBuffer(bitmap.width, bitmap.rows, 1, bitmap.pitch, bitmap.buffer);
        expandBitmap<PixelR8>(destX, destY, sampleBuffer, &bitmapBuffer);
    });

    // Mark the success.
    glyphConvertionSuccess[glyphIndex] = true;

    // Set the glyph metadata
    auto &metadata = glyphMetadata[glyphIndex];
    metadata.min = glm::vec2(destX/sampleScale + resultColumn*cellWidth, destY/sampleScale + resultRow*cellHeight);
    metadata.max = metadata.min + glm::vec2((width + sampleScale  - 1) / sampleScale, (height + sampleScale  - 1) / sampleScale);

    // Compute the metrics scale factor
    auto metricsScaleFactor = 1.0f / (64 * sampleScale);
    
    // Set the metrics
    auto &metrics = face->glyph->metrics;
    metadata.advance = glm::vec2(metrics.horiAdvance, metrics.vertAdvance)*metricsScaleFactor;
    metadata.size = glm::vec2(metrics.width, metrics.height)*metricsScaleFactor;
    metadata.horizontalBearing = glm::vec2(metrics.horiBearingX, metrics.horiBearingY)*metricsScaleFactor;
    metadata.verticalBearing = glm::vec2(metrics.vertBearingX, metrics.vertBearingY)*metricsScaleFactor;

}

template<typename FT>
void characterMapDo(const FT &f)
{
    FT_ULong charCode;
    FT_UInt glyphIndex;

    charCode = FT_Get_First_Char(face, &glyphIndex);
    while (glyphIndex != 0)
    {
        f(charCode, glyphIndex);
        charCode = FT_Get_Next_Char(face, charCode, &glyphIndex);
    }
}

void extractCharacterMap()
{
    characterMapDo([](int charCode, int glyphIndex) {
        // Ignore characters that could not be converted.
        if (!glyphConvertionSuccess[glyphIndex])
            return;

        LodenFontCharMapEntry entry;
        entry.character = charCode;
        entry.glyph = glyphIndex;
        characterMap.push_back(entry);
    });
}

bool writeFontMetadata(const std::string &metadataName)
{
    // Write the file.
    OutputStdFile out;
    if (!out.open(metadataName, true))
        return false;

    // Write the header
    LodenFontHeader header;
    memset(&header, 0, sizeof(header));
    memcpy(header.signature, LodenFontSignature, sizeof(header.signature));
    header.numberOfGlyphs = (uint32_t)glyphMetadata.size();
    header.numberOfCharMapEntries = (uint32_t)characterMap.size();
    header.cellWidth = cellWidth;
    header.cellHeight = cellHeight;
    if (distanceFieldFont)
        header.flags |= LodenFontFlags::SignedDistanceField;
    if (fwrite(&header, sizeof(header), 1, out.get()) != 1)
        return false;

    // Write the glyph metadata
    if (fwrite(&glyphMetadata[0], sizeof(LodenFontGlyphMetadata), glyphMetadata.size(), out.get()) != glyphMetadata.size())
        return false;

    // Write the character table
    if (fwrite(&characterMap[0], sizeof(LodenFontCharMapEntry), characterMap.size(), out.get()) != characterMap.size())
        return false;

    out.commit();
    return true;
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
        else if (!strcmp(argv[i], "-sdf"))
        {
            distanceFieldFont = true;
        }
        else if (!strcmp(argv[i], "-bitmap"))
        {
            distanceFieldFont = false;
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

    resultBuffer.reset(new LocalImageBuffer(outputWidth, outputHeight, 8, outputWidth));

    // Clear the result buffer.
    clearImageBuffer(resultBuffer.get());

    // Create the converter threads
    std::vector<GlyphConverter> converters(numberOfJobs);
    for (auto &converter : converters)
        converter.start();

    // Start converting the glyphs.
    glyphConvertionSuccess.resize(face->num_glyphs);
    glyphMetadata.resize(face->num_glyphs);
    for (int i = 0; i < face->num_glyphs; ++i)
    {
        printf("Converting glyph %05d / %05d\r", i, (int)face->num_glyphs);
        startGlyphConvertion(i, i / columns, i % columns);
    }

    if (failCount)
        printf("Failed to convert %d glyphs\n", failCount);

    // Extract the character map.
    extractCharacterMap();

    // Wait for the converters to end
    for (auto &converter : converters)
        converter.shutdown();

    saveImageAsPng(outputName + ".png", resultBuffer.get());
    writeFontMetadata(outputName + ".lodenfnt");

    FT_Done_Face(face);
    FT_Done_FreeType(ftLibrary);

    return 0;
}
