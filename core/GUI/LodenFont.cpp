#include "LodenFont.hpp"
#include "Loden/FileSystem.hpp"
#include "Loden/Stdio.hpp"
#include "Loden/GUI/LodenFontFormat.hpp"
#include "Loden/GUI/AgpuCanvas.hpp"
#include "Loden/Image/ReadWrite.hpp"
#include "Loden/Texture.hpp"
#include "Loden/PipelineStateManager.hpp"

#include <string.h>
#include <vector>
#include <unordered_map>

namespace Loden
{
namespace GUI
{

class LodenFontFace : public ObjectSubclass<LodenFontFace, FontFace>
{
    LODEN_OBJECT_TYPE(LodenFontFace);
public:
    LodenFontFace(Engine *engine = 0);
    ~LodenFontFace();

    virtual void release();

    virtual glm::vec2 drawCharacter(Canvas *canvas, int character, int pointSize, const glm::vec2 &position);
    virtual glm::vec2 drawUtf8(Canvas *canvas, const std::string &text, int pointSize, const glm::vec2 &position);
    virtual glm::vec2 drawUtf16(Canvas *canvas, const std::wstring &text, int pointSize, const glm::vec2 &position);

    virtual Rectangle computeUtf8TextRectangle(const std::string &text, int pointSize);
    virtual Rectangle computeUtf16TextRectangle(const std::wstring &text, int pointSize);

    bool read(FILE *in, Image::ImageBuffer *image);

private:
    float computeScaleFactor(int pointSize);
    int getGlyphForCharacter(int character);

    Rectangle computeDestinationRectangle(LodenFontGlyphMetadata &glyph, float scaleFactor, const glm::vec2 &position);
    glm::vec2 drawNextCharacter(Canvas *canvas, int character, int previousCharacter, int pointSize, const glm::vec2 &position);
    glm::vec2 appendCharacterBoundingBox(int character, int previousCharacter, int pointSize, const glm::vec2 &position, Rectangle &accumulatedBoundingBox);

    Engine *engine;

    float basePointSize;
    bool isSignedDistanceField;
    std::vector<LodenFontGlyphMetadata> glyphData;
    std::unordered_map<uint32_t, uint32_t> characterMap;
    TexturePtr texture;
    agpu_shader_resource_binding_ref textureBinding;
    glm::vec2 texcoordScale;
};

LodenFontFace::LodenFontFace(Engine *engine)
    : engine(engine)
{
}

LodenFontFace::~LodenFontFace()
{
}

void LodenFontFace::release()
{
}

float LodenFontFace::computeScaleFactor(int pointSize)
{
    return float(pointSize) / basePointSize;
}

int LodenFontFace::getGlyphForCharacter(int character)
{
    auto it = characterMap.find(character);
    if (it != characterMap.end())
        return it->second;
    return 0;
}

Rectangle LodenFontFace::computeDestinationRectangle(LodenFontGlyphMetadata &glyph, float scaleFactor, const glm::vec2 &position)
{
    glm::vec2 drawPosition = position;
    auto size = glyph.max - glyph.min;
    return Rectangle(drawPosition, drawPosition + size * scaleFactor);
}

glm::vec2 LodenFontFace::drawNextCharacter(Canvas *canvas, int character, int previousCharacter, int pointSize, const glm::vec2 &position)
{
    auto &glyph = glyphData[getGlyphForCharacter(character)];
    
    auto scaleFactor = computeScaleFactor(pointSize);

    Rectangle source(glyph.min*texcoordScale, glyph.max*texcoordScale);
    Rectangle dest = computeDestinationRectangle(glyph, scaleFactor, position);

    // Draw the character
    canvas->drawBitmapCharacter(dest, source);

    return position + glm::vec2(glyph.advance.x*scaleFactor, 0);
}

glm::vec2 LodenFontFace::appendCharacterBoundingBox(int character, int previousCharacter, int pointSize, const glm::vec2 &position, Rectangle &accumulatedBoundingBox)
{
    auto &glyph = glyphData[getGlyphForCharacter(character)];
    auto scaleFactor = computeScaleFactor(pointSize);

    Rectangle rect = computeDestinationRectangle(glyph, scaleFactor, position);
    accumulatedBoundingBox.insertRectangle(rect);

    return position + glm::vec2(glyph.advance.x*scaleFactor, 0);
}

glm::vec2 LodenFontFace::drawCharacter(Canvas *canvas, int character, int pointSize, const glm::vec2 &position)
{
    canvas->beginBitmapTextDrawing(textureBinding.get(), isSignedDistanceField);
    auto result = drawNextCharacter(canvas, character, -1, pointSize, position);
    canvas->endBitmapTextDrawing();
    return result;
}

glm::vec2 LodenFontFace::drawUtf8(Canvas *canvas, const std::string &text, int pointSize, const glm::vec2 &position)
{
    // TODO: Decode the UTF-8 character
    auto currentPosition = position;
    canvas->beginBitmapTextDrawing(textureBinding.get(), isSignedDistanceField);
    int previousCharacter = -1;
    for (size_t i = 0; i < text.size(); ++i)
    {
        int character = text[i];
        currentPosition = drawNextCharacter(canvas, character, previousCharacter, pointSize, currentPosition);
        previousCharacter = character;
    }
    canvas->endBitmapTextDrawing();

    return currentPosition;
}

glm::vec2 LodenFontFace::drawUtf16(Canvas *canvas, const std::wstring &text, int pointSize, const glm::vec2 &position)
{
    return position;
}

Rectangle LodenFontFace::computeUtf8TextRectangle(const std::string &text, int pointSize)
{
    int previousCharacter = -1;
    glm::vec2 currentPosition(0);
    Rectangle boundingBox(glm::vec2(0, 0), glm::vec2(0, 0));
    for (size_t i = 0; i < text.size(); ++i)
    {
        int character = text[i];
        currentPosition = appendCharacterBoundingBox(character, previousCharacter, pointSize, currentPosition, boundingBox);
        previousCharacter = character;
    }

    return boundingBox;
}

Rectangle LodenFontFace::computeUtf16TextRectangle(const std::wstring &text, int pointSize)
{
    return Rectangle();
}

bool LodenFontFace::read(FILE *in, Image::ImageBuffer *image)
{
    if (image->getBitsPerPixel() != 8)
        return false;

    LodenFontHeader header;
    if (fread(&header, sizeof(header), 1, in) != 1)
        return false;

    // Check the signature
    if (memcmp(header.signature, LodenFontSignature, sizeof(header.signature)) != 0)
        return false;

    basePointSize = header.pointSize;
    isSignedDistanceField = (header.flags & LodenFontFlags::SignedDistanceField) != 0;

    // Read the glyph metadata.
    glyphData.resize(header.numberOfGlyphs);
    if (fread(&glyphData[0], sizeof(LodenFontGlyphMetadata), glyphData.size(), in) != glyphData.size())
        return false;

    // Read the character map
    std::vector<LodenFontCharMapEntry> charMapEntries(header.numberOfCharMapEntries);
    if (fread(&charMapEntries[0], sizeof(LodenFontCharMapEntry), charMapEntries.size(), in) != charMapEntries.size())
        return false;

    // Insert the character map entries into the hash table.
    characterMap.reserve(charMapEntries.size());
    for (auto &entry : charMapEntries)
        characterMap.insert(std::make_pair(entry.character, entry.glyph));

    // Create the texture for the image.
    auto format = isSignedDistanceField ? AGPU_TEXTURE_FORMAT_R8_SNORM : AGPU_TEXTURE_FORMAT_R8_UNORM;
    texture = Texture::createFromImage(engine, image, format);
    if (!texture)
        return false;

    // Create the texture binding.
    auto shaderSignature = engine->getPipelineStateManager()->getShaderSignature("GUI");
    if (!shaderSignature)
        return false;

    textureBinding = shaderSignature->createShaderResourceBinding(2);
    if (!textureBinding)
        return false;

    // Bind the texture.
    textureBinding->bindTexture(0, texture->getHandle().get(), 0, -1, 0.0);

    // Compute the texcoord scale factor
    texcoordScale = glm::vec2(1.0f / image->getWidth(), 1.0f / image->getHeight());
    return true;
}

LodenFontLoader::LodenFontLoader(Engine *engine)
    : engine(engine)
{

}
LodenFontLoader::~LodenFontLoader()
{
}

bool LodenFontLoader::initialize()
{
    return true;
}

void LodenFontLoader::shutdown()
{
}

bool LodenFontLoader::canLoadFaceFromFile(const std::string &fileName)
{
    return extensionOfPath(fileName) == ".lodenfnt";
}

FontFacePtr LodenFontLoader::loadFaceFromFile(const std::string &fileName)
{
    std::string imageFileName = removeExtension(fileName) + ".png";
    auto image = Image::loadImageFromPng(imageFileName);

    InputStdFile in;
    if (!in.open(fileName))
        return nullptr;

    auto result = std::make_shared<LodenFontFace> (engine);
    if (!result->read(in.get(), image.get()))
        return nullptr;

    return result;
}

} // End of namespace GUI
} // End of namespace Loden
