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
#include <condition_variable>

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
static int pointSize = 14;
static int sampleScale = 4;
static float distanceScale = 2.0f;
static int margin = 1;
static int atlasWidth = 2048;
static int atlasHeight;
static int numberOfJobs = 1;
static int failCount = 0;
static bool distanceFieldFont = false;
static bool unsignedValues = true;
static std::vector<bool> glyphConvertionSuccess;
static std::vector<std::shared_ptr<LocalImageBuffer>> glyphConvertionResults;

static FT_Library ftLibrary;
static FT_Face face;
static FT_Face downSampledFace;

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

        sampleWidth = 0;
        sampleHeight = 0;
        resultWidth = 0;
        resultHeight = 0;
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
    void beginConvertion(int glyphIndex,
        int sampleWidth, int sampleHeight,
        int resultWidth, int resultHeight,
        const FT &function)
    {
        std::unique_lock<std::mutex> l(controlMutex);
        this->glyphIndex = glyphIndex;
        createBuffers(sampleWidth, sampleHeight, resultWidth, resultHeight);
        function(sampleBuffer.get());
        hasConvertionJob = true;
        hasJobCondition.notify_all();
    }

private:
    void convert();
    void converterThread();

    void createBuffers(int sampleWidth, int sampleHeight, int resultWidth, int resultHeight)
    {
        if(this->sampleWidth != sampleWidth || this->sampleHeight != sampleHeight)
        {
            this->sampleWidth = sampleWidth;
            this->sampleHeight = sampleHeight;

            auto samplePitch = (sampleWidth + 3) / 4 * 4;
            sampleBuffer.reset(new LocalImageBuffer(sampleWidth, sampleHeight, 8, samplePitch));
            distanceTransformBuffer.reset(new LocalImageBuffer(sampleWidth, sampleHeight, 8, samplePitch));
            downsampleBuffer.reset(new DoubleImageBuffer(sampleWidth, sampleHeight, 8, samplePitch));
        }

        this->resultWidth = resultWidth;
        this->resultHeight = resultHeight;
        glyphResultBuffer = std::make_shared<LocalImageBuffer> (resultWidth, resultHeight, 8, (resultWidth + 3) / 4 * 4);
    }

    int glyphIndex;
    int sampleWidth;
    int sampleHeight;
    int resultWidth;
    int resultHeight;
    bool hasConvertionJob;
    bool shuttingDown;

    std::unique_ptr<LocalImageBuffer> sampleBuffer;
    std::unique_ptr<LocalImageBuffer> distanceTransformBuffer;
    std::unique_ptr<DoubleImageBuffer> downsampleBuffer;
    std::shared_ptr<LocalImageBuffer> glyphResultBuffer;

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
        computeSmallerDistanceField<PixelR8s>(glyphResultBuffer.get(), sampleBuffer.get(), distanceScale);

        if(unsignedValues)
            signedToUnsignedPixels<PixelR8, PixelR8s> (glyphResultBuffer.get(), glyphResultBuffer.get());
    }
    else
    {
        // Just downsample.
        if(sampleScale > 1)
        {
            if(sampleWidth != resultWidth * sampleScale || sampleHeight != resultHeight * sampleScale)
            {
                auto factor = sampleScale / 2;
                downsample<PixelR8> (downsampleBuffer.get(), sampleBuffer.get(), factor);
                linearScale<PixelR8> (glyphResultBuffer.get(), resultWidth, resultHeight,
                    downsampleBuffer.get(), floor(float(sampleWidth) / factor), floor(float(sampleHeight) / factor));
            }
            else
            {
                // Perfect downsampling.
                downsample<PixelR8>(downsampleBuffer.get(), sampleBuffer.get(), sampleScale);
                copyRectangle<PixelR8s> (0, 0, glyphResultBuffer.get(), 0, 0, resultWidth, resultHeight, downsampleBuffer.get());
            }
        }
        else
        {
            copyRectangle<PixelR8s> (0, 0, glyphResultBuffer.get(), 0, 0, resultWidth, resultHeight, sampleBuffer.get());
        }
    }

    // Copy to the result
    glyphConvertionResults[glyphIndex] = glyphResultBuffer;
}

template<typename FT>
void queueInConverter(int glyphIndex,
    int sampleWidth, int sampleHeight,
    int resultWidth, int resultHeight,
    const FT &function)
{
    GlyphConverter *converter;
    {
        std::unique_lock<std::mutex> l(converterWaitingQueueMutex);

        while (converterWaitingQueue.empty())
            converterWaitingQueueCondition.wait(l);

        converter = converterWaitingQueue.front();
        converterWaitingQueue.pop();
    }

    converter->beginConvertion(glyphIndex,
        sampleWidth, sampleHeight,
        resultWidth, resultHeight,
        function);
}

void startGlyphConvertion(int glyphIndex)
{
    auto error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
    auto error2 = FT_Load_Glyph(downSampledFace, glyphIndex, FT_LOAD_DEFAULT);
    if (error || error2)
    {
        ++failCount;
        printWarning("\nFailed to load the glyph %d.\n", glyphIndex);
        return;
    }

    error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
    error2 = FT_Render_Glyph(downSampledFace->glyph, FT_RENDER_MODE_MONO);
    if (error || error2)
    {
        ++failCount;
        printWarning("Failed to render the glyph %d.\n", glyphIndex);
        return;
    }

    auto &bitmap = face->glyph->bitmap;
    auto sampleWidth = face->glyph->bitmap.width;
    auto sampleHeight = face->glyph->bitmap.rows;
    auto glyphWidth = downSampledFace->glyph->bitmap.width;
    auto glyphHeight = downSampledFace->glyph->bitmap.rows;

    // Convert the bitmap into single byte image.
    queueInConverter(glyphIndex,
        sampleWidth, sampleHeight,
        glyphWidth, glyphHeight,
        [&](ImageBuffer *sampleBuffer) {
        clearImageBuffer(sampleBuffer);
        ExternalImageBuffer bitmapBuffer(bitmap.width, bitmap.rows, 1, bitmap.pitch, bitmap.buffer);
        expandBitmap<PixelR8>(0, 0, sampleBuffer, &bitmapBuffer);
    });

    // Mark the success.
    glyphConvertionSuccess[glyphIndex] = true;

    // Set the glyph metadata
    auto &metadata = glyphMetadata[glyphIndex];
    metadata.min = glm::vec2(0, 0);
    metadata.max = glm::vec2(downSampledFace->glyph->bitmap.width, downSampledFace->glyph->bitmap.rows);

    // Compute the metrics scale factor
    auto metricsScaleFactor = 1.0f / (64 * sampleScale);

    // Set the metrics
    auto &metrics = downSampledFace->glyph->metrics;
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
    header.pointSize = pointSize;
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
        else if (!strcmp(argv[i], "-pointSize"))
        {
            pointSize = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-sampleScale"))
        {
            sampleScale = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-help"))
        {
            printHelp();
            return 0;
        }
        else if (!strcmp(argv[i], "-margin"))
        {
            margin = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-distanceField"))
        {
            distanceFieldFont = true;
        }
        else if(!strcmp(argv[i], "-unsigned"))
        {
            unsignedValues = true;
        }
        else if(!strcmp(argv[i], "-signed"))
        {
            unsignedValues = false;
        }
        else if (!strcmp(argv[i], "-bitmap"))
        {
            distanceFieldFont = false;
        }
        else if(!strcmp(argv[i], "-j"))
        {
            numberOfJobs = atoi(argv[++i]);
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

    auto error = FT_Init_FreeType(&ftLibrary);
    if (error)
    {
        fprintf(stderr, "Failed to initialize freetype.\n");
        return -1;
    }

    error = FT_New_Face(ftLibrary, inputFileName.c_str(), 0, &face);
    auto error2 = FT_New_Face(ftLibrary, inputFileName.c_str(), 0, &downSampledFace);
    if (error == FT_Err_Unknown_File_Format || error2 == FT_Err_Unknown_File_Format)
    {
        fprintf(stderr, "Unsupported font format.\n");
        return -1;
    }
    else if (error || error2)
    {
        fprintf(stderr, "Failed to load the font.\n");
        return -1;
    }

    error = FT_Set_Char_Size(face, (pointSize*sampleScale) << 6, 0, 0, 0);
    error2 = FT_Set_Char_Size(downSampledFace, pointSize << 6, 0, 0, 0);
    if (error || error2)
    {
        fprintf(stderr, "Failed to set the font face size.\n");
        return -1;
    }

    // Get the number of glyphs.
    int numberOfGlyphs = face->num_glyphs;
    printf("Number of available glyphs: %d\n", numberOfGlyphs);

    // Create the converter threads
    std::vector<GlyphConverter> converters(numberOfJobs);
    for (auto &converter : converters)
        converter.start();

    // Start converting the glyphs.
    glyphConvertionResults.resize(numberOfGlyphs);
    glyphConvertionSuccess.resize(numberOfGlyphs);
    glyphMetadata.resize(numberOfGlyphs);
    for (int i = 0; i < numberOfGlyphs; ++i)
    {
        printf("Converting glyph %05d / %05d\r", i, (int)numberOfGlyphs);
        startGlyphConvertion(i);
    }

    if (failCount)
        printf("Failed to convert %d glyphs\n", failCount);

    // Extract the character map.
    extractCharacterMap();

    // Wait for the converters to end
    for (auto &converter : converters)
        converter.shutdown();

    // Distribute the glyphs.
    glm::vec2 glyphPosition(margin, margin);
    float maxHeight = 0;
    for (int i = 0; i < numberOfGlyphs; ++i)
    {
        auto &glyph = glyphMetadata[i];
        auto extent = glyph.max - glyph.min;
        auto min = glyphPosition;
        auto max = glyphPosition + extent;
        if(max.x + margin >= atlasWidth)
        {
            glyphPosition = glm::vec2(margin, glyphPosition.y + margin + maxHeight);
            min = glyphPosition;
            max = glyphPosition + extent;
            maxHeight = 0;
        }

        glyph.min = min;
        glyph.max = max;

        glyphPosition.x += margin + extent.x;
        maxHeight = std::max(maxHeight, extent.y);
    }

    atlasHeight = sameOrNextPowerOfTwo(ceil(glyphPosition.y + margin + maxHeight));
    printf("Atlas extent: %d %d\n", atlasWidth, atlasHeight);

    // Clear the result buffer.
    resultBuffer.reset(new LocalImageBuffer(atlasWidth, atlasHeight, 8, atlasWidth));
    clearImageBuffer(resultBuffer.get());

    // Copy the glyphs into the result buffer.
    for (int i = 0; i < numberOfGlyphs; ++i)
    {
        auto &glyphMeta = glyphMetadata[i];
        auto &glyph = glyphConvertionResults[i];
        if(!glyph)
            continue;

        copyRectangle<PixelR8s> (glyphMeta.min.x, glyphMeta.min.y, resultBuffer.get(), 0, 0, glyph->getWidth(), glyph->getHeight(), glyph.get());
    }

    saveImageAsPng(outputName + ".png", resultBuffer.get());
    writeFontMetadata(outputName + ".lodenfnt");

    FT_Done_Face(face);
    FT_Done_FreeType(ftLibrary);

    return 0;
}
