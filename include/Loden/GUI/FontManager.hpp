#ifndef LODEN_FONT_MANAGER_HPP
#define LODEN_FONT_MANAGER_HPP

#include "Loden/Common.hpp"
#include "Loden/GUI/Font.hpp"
#include "Loden/Engine.hpp"

namespace Loden
{
namespace GUI
{
LODEN_DECLARE_CLASS(FontManager);
LODEN_DECLARE_CLASS(FreeTypeFontLoader);

/**
 * Font manager
 */
class LODEN_CORE_EXPORT FontManager
{
public:
    FontManager(Engine *engine);
    ~FontManager();

    bool initialize();
    void shutdown();

    void addFont(const std::string &name, const FontPtr &font);
    FontPtr getFont(const std::string &name);

    FontPtr getDefaultFont() const;
    FontPtr getDefaultSansFont() const;
    FontPtr getDefaultSerifFont() const;
    FontPtr getDefaultMonospaceFont() const;

private:
    typedef std::map<std::string, FontPtr> Fonts;
    bool loadFontsFromFile(const std::string &fontsDescriptionFileName);

    Engine *engine;
    FreeTypeFontLoaderPtr fontLoader;

    FontPtr defaultFont;
    FontPtr defaultSerifFont;
    FontPtr defaultSansFont;
    FontPtr defaultMononospacedFont;
    Fonts fonts;
};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_FONT_MANAGER_HPP
