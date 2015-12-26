#include "FreeTypeFont.hpp"
#include <glm/mat3x3.hpp>
#include "Loden/Printing.hpp"
#include "Loden/GUI/Canvas.hpp"
#include FT_OUTLINE_H

namespace Loden
{
namespace GUI
{

inline glm::vec2 convertFreeTypeVector(const FT_Vector *vector)
{
    return glm::vec2(vector->x, vector->y);
}

/**
 * Free type font face wrapper.
 */
class FreeTypeFace: public FontFace
{
public:
    FreeTypeFace(FT_Face face)
        : face(face)
    {
        currentPointSize = -1;
        hasKerning = FT_HAS_KERNING(face);
    }

    ~FreeTypeFace()
    {
        release();
    }

    void release()
    {
        if (face)
            FT_Done_Face(face);
        face = nullptr;
    }

    glm::vec2 drawNextCharacter(Canvas *canvas, int character, int previousCharacter, int pointSize, const glm::vec2 &position);

    virtual glm::vec2 drawCharacter(Canvas *canvas, int character, int pointSize, const glm::vec2 &position);
    virtual glm::vec2 drawUtf8(Canvas *canvas, const std::string &text, int pointSize, const glm::vec2 &position);
    virtual glm::vec2 drawUtf16(Canvas *canvas, const std::wstring &text, int pointSize, const glm::vec2 &position);

private:
    void drawOutline(Canvas *canvas, FT_Outline *outline, const glm::vec2 &scale, const glm::vec2 &position);
    bool updatePointSize(int newPointSize);

    FT_Face face;
    int currentPointSize;
    bool hasKerning;
};

bool FreeTypeFace::updatePointSize(int newPointSize)
{
    if (currentPointSize == newPointSize)
        return true;

    auto error = FT_Set_Char_Size(face, 0, newPointSize * 64, 72, 72);
    if (error)
    {
        printError("Failed to set the point size of a font face.");
        return false;
    }
    
    currentPointSize = newPointSize;
    return true;
}

glm::vec2 FreeTypeFace::drawNextCharacter(Canvas *canvas, int character, int previousCharacter, int pointSize, const glm::vec2 &position)
{
    const auto ScaleFactor = 1.0f / 64.0f;

    // Get the glyph index.
    auto glyphIndex = FT_Get_Char_Index(face, character);

    // Load the glyph.
    auto error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_BITMAP);
    if (error)
        return position;
    auto glyph = face->glyph;

    // Only support outline fonts.
    if (glyph->format != FT_GLYPH_FORMAT_OUTLINE)
        return position;

    // Get the glyph metrics.
    auto &metrics = glyph->metrics;
    auto bearingY = metrics.horiBearingY * ScaleFactor;
    auto advance = metrics.horiAdvance*ScaleFactor;

    // Get the kerning.
    auto kerning = 0.0f;
    if (hasKerning && previousCharacter >= 0 && glyphIndex != 0)
    {
        auto previousGlyph = FT_Get_Char_Index(face, previousCharacter);
        if (previousGlyph != 0)
        {
            FT_Vector delta;
            FT_Get_Kerning(face, previousGlyph, glyphIndex, FT_KERNING_DEFAULT, &delta);
            kerning = delta.x * ScaleFactor;
        }
    }

    // Draw the outline.
    auto scale = glm::vec2(ScaleFactor, -ScaleFactor);
    auto translation = position + glm::vec2(0, bearingY);
    drawOutline(canvas, &glyph->outline, scale, position);

    // Compute the advance.
    return position + glm::vec2(advance + kerning, 0);
}

void FreeTypeFace::drawOutline(Canvas *canvas, FT_Outline *outline, const glm::vec2 &scale, const glm::vec2 &translation)
{
    struct DrawState
    {
        Canvas *canvas;
        glm::vec2 scale;
        glm::vec2 translation;
    };

    FT_Outline_Funcs funcs;
    memset(&funcs, 0, sizeof(funcs));
    funcs.move_to = [](const FT_Vector *to, void *user) {
        auto state = reinterpret_cast<DrawState*> (user);
        auto transformedTo = convertFreeTypeVector(to) * state->scale + state->translation;
        state->canvas->moveTo(transformedTo);
        return 0;
    };
    funcs.line_to = [](const FT_Vector *to, void *user) {
        auto state = reinterpret_cast<DrawState*> (user);
        auto transformedTo = convertFreeTypeVector(to) * state->scale + state->translation;
        state->canvas->lineTo(transformedTo);
        return 0;
    };
    funcs.conic_to = [](const FT_Vector *control, const FT_Vector *to, void *user) {
        auto state = reinterpret_cast<DrawState*> (user);
        auto transformedControl = convertFreeTypeVector(control) * state->scale + state->translation;
        auto transformedTo = convertFreeTypeVector(to) * state->scale + state->translation;
        state->canvas->quadTo(transformedControl, transformedTo);
        return 0;
    };
    funcs.cubic_to = [](const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user) {
        auto state = reinterpret_cast<DrawState*> (user);
        auto transformedControl1 = convertFreeTypeVector(control1) * state->scale + state->translation;
        auto transformedControl2 = convertFreeTypeVector(control2) * state->scale + state->translation;
        auto transformedTo = convertFreeTypeVector(to) * state->scale + state->translation;
        state->canvas->cubicTo(transformedControl1, transformedControl2, transformedTo);
        return 0;
    };

    DrawState state;
    state.canvas = canvas;
    state.scale = scale;
    state.translation = translation;
    FT_Outline_Decompose(outline, &funcs, &state);
}

glm::vec2 FreeTypeFace::drawCharacter(Canvas *canvas, int character, int pointSize, const glm::vec2 &position)
{
    if (!updatePointSize(pointSize))
        return position;

    canvas->beginFillPath(PathFillRule::NonZero);
    auto result = drawNextCharacter(canvas, character, -1, pointSize, position);
    canvas->endFillPath();
    return result;
}

glm::vec2 FreeTypeFace::drawUtf8(Canvas *canvas, const std::string &text, int pointSize, const glm::vec2 &position)
{
    if (!updatePointSize(pointSize))
        return position;

    // TODO: Decode the UTF-8 character
    auto currentPosition = position;
    canvas->beginFillPath(PathFillRule::NonZero);
    int previousCharacter = -1;
    for (size_t i = 0; i < text.size(); ++i)
    {
        int character = text[i];
        currentPosition = drawNextCharacter(canvas, character, previousCharacter, pointSize, currentPosition);
        previousCharacter = character;
    }
    canvas->endFillPath();

    return currentPosition;
}

glm::vec2 FreeTypeFace::drawUtf16(Canvas *canvas, const std::wstring &text, int pointSize, const glm::vec2 &position)
{
    if (!updatePointSize(pointSize))
        return position;

    // TODO: Decode the UTF-16 character
    auto currentPosition = position;
    canvas->beginFillPath(PathFillRule::NonZero);
    int previousCharacter = -1;
    for (size_t i = 0; i < text.size(); ++i)
    {
        int character = text[i];
        currentPosition = drawNextCharacter(canvas, character, previousCharacter, pointSize, currentPosition);
        previousCharacter = character;
    }
    canvas->endFillPath();

    return currentPosition;
}

FreeTypeFontLoader::FreeTypeFontLoader(Engine *engine)
    : engine(engine)
{
}

FreeTypeFontLoader::~FreeTypeFontLoader()
{
}

bool FreeTypeFontLoader::initialize()
{
    auto error = FT_Init_FreeType(&library);
    if (error)
    {
        printError("Failed to initialize the free type library.");
        return false;
    }

    return true;
}

void FreeTypeFontLoader::shutdown()
{
    FT_Done_FreeType(library);
}

FontFacePtr FreeTypeFontLoader::loadFaceFromFile(const std::string &fileName)
{
    FT_Face face;
    auto error = FT_New_Face(library, fileName.c_str(), 0, &face);
    if (error)
    {
        printError("Failed to load font face file %s", fileName.c_str());
        return nullptr;
    }

    return std::make_shared<FreeTypeFace> (face);
}

} // End of namespace GUI
} // End of namespace Loden
