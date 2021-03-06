#ifndef LODEN_FREETYPE_FONT_HPP
#define LODEN_FREETYPE_FONT_HPP

#include "Loden/Engine.hpp"
#include "Loden/GUI/Font.hpp"
#include "Loden/GUI/FontManager.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace Loden
{
namespace GUI
{

/**
 * Free type font loader
 */
class FreeTypeFontLoader: public ObjectSubclass<FreeTypeFontLoader, FontLoader>
{
public:
    FreeTypeFontLoader(Engine *engine=nullptr);
    ~FreeTypeFontLoader();

    bool initialize();
    void shutdown();

    bool canLoadFaceFromFile(const std::string &fileName);
    FontFacePtr loadFaceFromFile(const std::string &fileName);

private:
    Engine *engine;
    FT_Library library;
};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_FREETYPE_FONT_HPP
