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
class LODEN_CORE_EXPORT MenuBar : public ObjectSubclass<MenuBar, Widget>
{
    LODEN_OBJECT_TYPE(MenuBar);
public:
    MenuBar(const SystemWindowPtr &systemWindow = nullptr);
    ~MenuBar();

    static MenuBarPtr create(const SystemWindowPtr &systemWindow);

    void addItem(const MenuItemPtr &item);
    void addMenu(const std::string &text, const MenuPtr &menu);
    void addAction(const std::string &text, const ActionEventHandler &action);

    virtual glm::vec2 getMinimalSize();

public:
    virtual void drawContentOn(Canvas *canvas) override;

    virtual void handleMouseButtonDown(MouseButtonEvent &event) override;
    virtual void handleMouseButtonUp(MouseButtonEvent &event) override;
    virtual void handleMouseMotion(MouseMotionEvent &event) override;

    virtual void handlePopUpsKilledEvent(PopUpsKilledEvent &event) override;

private:
    void activateMenuAtPosition(const glm::vec2 &position);

    MenuItemPtr getItemAtPosition(const glm::vec2 &position, int *itemIndex, glm::vec2 *itemPosition);

    std::vector<MenuItemPtr> items;
    bool activated;
    int activeItemIndex;

};
} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_BUTTON_HPP
