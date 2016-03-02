#ifndef LODEN_FONT_MANAGER_HPP
#define LODEN_FONT_MANAGER_HPP

#include "Loden/Object.hpp"
#include "Loden/GUI/Font.hpp"
#include "Loden/Engine.hpp"
#include <vector>

namespace Loden
{
namespace GUI
{
LODEN_DECLARE_CLASS(FontManager);
LODEN_DECLARE_INTERFACE(FontLoader);

struct LODEN_CORE_EXPORT FontLoader: public ObjectInterfaceSubclass<FontLoader, Object>
{
    LODEN_OBJECT_TYPE(FontLoader);

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    virtual bool canLoadFaceFromFile(const std::string &fileName) = 0;
    virtual FontFacePtr loadFaceFromFile(const std::string &fileName) = 0;
};

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
    FontFacePtr loadFaceFromFile(const std::string &fileName);

    Engine *engine;
    std::vector<FontLoaderPtr> fontLoaders;

    FontPtr defaultFont;
    FontPtr defaultSerifFont;
    FontPtr defaultSansFont;
    FontPtr defaultMononospacedFont;
    Fonts fonts;
};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_FONT_MANAGER_HPP
