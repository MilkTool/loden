#ifndef LODEN_GUI_MENU_BAR_HPP
#define LODEN_GUI_MENU_BAR_HPP

#include "Loden/GUI/Widget.hpp"
#include "Loden/GUI/Menu.hpp"
#include <vector>

namespace Loden
{
namespace GUI
{

LODEN_DECLARE_CLASS(MenuBar);

/**
* Menu bar
*/
class LODEN_CORE_EXPORT MenuBar : public Widget
{
public:
    MenuBar();
    ~MenuBar();

    static MenuBarPtr create();

    void addMenu(const std::string &text, const MenuPtr &menu);

    virtual glm::vec2 getMinimalSize();

    virtual void drawContentOn(Canvas *canvas);

private:
    struct Item
    {
        Item(const std::string &text, const MenuPtr &menu)
            : text(text), menu(menu) {}

        std::string text;
        MenuPtr menu;
        Rectangle rectangle;
    };

    std::vector<Item> items;
};
} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_BUTTON_HPP
