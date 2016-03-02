#ifndef LODEN_GUI_LODEN_FONT_HPP
#define LODEN_GUI_LODEN_FONT_HPP

#include "Loden/Engine.hpp"
#include "Loden/GUI/Font.hpp"
#include "Loden/GUI/FontManager.hpp"

namespace Loden
{
namespace GUI
{

/**
* Free type font loader
*/
class LodenFontLoader : public ObjectSubclass<LodenFontLoader, FontLoader>
{
public:
    LodenFontLoader(Engine *engine=nullptr);
    ~LodenFontLoader();

    bool initialize();
    void shutdown();

    bool canLoadFaceFromFile(const std::string &fileName);
    FontFacePtr loadFaceFromFile(const std::string &fileName);

private:
    Engine *engine;
};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_LODEN_FONT_HPP
