#ifndef LODEN_GUI_MENU_HPP
#define LODEN_GUI_MENU_HPP

#include "Loden/GUI/Widget.hpp"
#include "Loden/GUI/Menu.hpp"

namespace Loden
{
namespace GUI
{

LODEN_DECLARE_CLASS(Menu);

/**
* Menu bar
*/
class LODEN_CORE_EXPORT Menu: public Widget
{
public:
    typedef EventSocket<ActionEvent>::EventHandler ActionEventHandler;

    Menu(const SystemWindowPtr &systemWindow);
    ~Menu();

    static MenuPtr create(const SystemWindowPtr &systemWindow);

    void addItem(const std::string &text, const ActionEventHandler &action);

    virtual glm::vec2 getMinimalSize();

    virtual void drawContentOn(Canvas *canvas);

private:

};
} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_MENU_HPP
