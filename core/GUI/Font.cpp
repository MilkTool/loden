#include "Loden/GUI/Font.hpp"

namespace Loden
{
namespace GUI
{

Font::Font()
{
}

Font::~Font()
{

}

void Font::release()
{
    for (auto &face : faces)
        face.second->release();

    faces.clear();
}

void Font::addFace(const std::string &name, const FontFacePtr &face)
{
    faces.insert(std::make_pair(name, face));
}

const Font::Faces &Font::getFaces() const
{
    return faces;
}

FontFacePtr Font::getFace(const std::string &name)
{
    auto it = faces.find(name);
    if (it != faces.end())
        return it->second;
    return nullptr;
}

void Font::loadSpecialFaces()
{
    defaultFace = getFace("default");
    boldFace = getFace("bold");
    boldItalicFace = getFace("bold-italic");
    italicFace = getFace("italic");
}

const FontFacePtr &Font::getDefaultFace() const
{
    return defaultFace;
}

const FontFacePtr &Font::getBoldFace() const
{
    return boldFace;
}

const FontFacePtr &Font::getBoldItalicFace() const
{
    return boldItalicFace;
}

const FontFacePtr &Font::getItalicFace() const
{
    return italicFace;
}

} // End of namespace GUI
} // End of namespace Loden
