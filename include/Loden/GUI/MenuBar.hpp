#ifndef LODEN_GUI_MENU_BAR_HPP
#define LODEN_GUI_MENU_BAR_HPP

#include "Loden/GUI/Widget.hpp"

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

    virtual glm::vec2 getMinimalSize();

    virtual void drawContentOn(Canvas *canvas);

private:

};
} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_BUTTON_HPP
