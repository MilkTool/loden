#ifndef LODEN_FONT_HPP
#define LODEN_FONT_HPP

#include "Loden/Common.hpp"
#include "Loden/Object.hpp"
#include <glm/vec2.hpp>
#include <map>
#include <string>

namespace Loden
{
namespace GUI
{
LODEN_DECLARE_CLASS(Canvas);
LODEN_DECLARE_CLASS(Font);
LODEN_DECLARE_CLASS(FontFace);


/**
 * A font is a container of font faces
 */
class LODEN_CORE_EXPORT Font
{
public:
    typedef std::map<std::string, FontFacePtr> Faces;

    Font();
    ~Font();

    void addFace(const std::string &name, const FontFacePtr &face);
    void release();

    const Faces &getFaces() const;
    FontFacePtr getFace(const std::string &name);

    void loadSpecialFaces();
    const FontFacePtr &getDefaultFace() const;
    const FontFacePtr &getBoldFace() const;
    const FontFacePtr &getBoldItalicFace() const;
    const FontFacePtr &getItalicFace() const;

private:
    FontFacePtr defaultFace;
    FontFacePtr boldFace;
    FontFacePtr boldItalicFace;
    FontFacePtr italicFace;
    Faces faces;
};

/**
 * A font face
 */
class LODEN_CORE_EXPORT FontFace : public Object
{
public:
    virtual void release() = 0;

    virtual glm::vec2 drawCharacter(Canvas *canvas, int character, int pointSize, const glm::vec2 &position) = 0;
    virtual glm::vec2 drawUtf8(Canvas *canvas, const std::string &text, int pointSize, const glm::vec2 &position) = 0;
    virtual glm::vec2 drawUtf16(Canvas *canvas, const std::wstring &text, int pointSize, const glm::vec2 &position) = 0;
};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_FONT_HPP
