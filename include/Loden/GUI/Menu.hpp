#ifndef LODEN_GUI_MENU_HPP
#define LODEN_GUI_MENU_HPP

#include "Loden/GUI/Widget.hpp"
#include "Loden/GUI/MenuItem.hpp"
#include <vector>

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
    LODEN_WIDGET_TYPE(Menu, Widget);
public:
    Menu(const SystemWindowPtr &systemWindow);
    ~Menu();

    static MenuPtr create(const SystemWindowPtr &systemWindow);

    void addItem(const MenuItemPtr &item);
    void addMenu(const std::string &text, const MenuPtr &menu);
    void addAction(const std::string &text, const ActionEventHandler &action);

public:
    virtual void drawContentOn(Canvas *canvas) override;

    virtual void handleAddedToParent(ParentChangedEvent &event) override;

    virtual void handleMouseButtonDown(MouseButtonEvent &event) override;
    virtual void handleMouseButtonUp(MouseButtonEvent &event) override;
    virtual void handleMouseMotion(MouseMotionEvent &event) override;

private:
    MenuItemPtr getItemAtPosition(const glm::vec2 &position, int *itemIndex, glm::vec2 *itemPosition);

    std::vector<MenuItemPtr> items;
    int activeItemIndex;

};
} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_MENU_HPP
