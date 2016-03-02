#ifndef LODEN_GUI_LODEN_FONT_FORMAT_HPP
#define LODEN_GUI_LODEN_FONT_FORMAT_HPP

#include <glm/vec2.hpp>

namespace Loden
{
namespace GUI
{

static constexpr const char *LodenFontSignature = "LODENFNT";

namespace LodenFontFlags
{
    enum Values
    {
        None = 0,
        SignedDistanceField = 1
    };
}

struct LodenFontHeader
{
    uint8_t signature[8];
    uint32_t numberOfGlyphs;
    uint32_t numberOfCharMapEntries;
    uint32_t cellWidth;
    uint32_t cellHeight;
    uint32_t flags;
};

/**
* Glyph metadata.
*/
struct LodenFontGlyphMetadata
{
    glm::vec2 min;
    glm::vec2 max;

    glm::vec2 advance;
    glm::vec2 size;
    glm::vec2 horizontalBearing;
    glm::vec2 verticalBearing;
};

struct LodenFontCharMapEntry
{
    int32_t character;
    int32_t glyph;
};


} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_LODEN_FONT_FORMAT_HPP
