#ifndef LODEN_GUI_MENU_ITEM_HPP
#define LODEN_GUI_MENU_ITEM_HPP

#include "Loden/GUI/Widget.hpp"
#include <string>

namespace Loden
{
namespace GUI
{

LODEN_DECLARE_CLASS(MenuItem);
LODEN_DECLARE_CLASS(Menu);
LODEN_DECLARE_CLASS(MenuBar);

/**
 * Menu item
 */
class LODEN_CORE_EXPORT MenuItem: public ObjectSubclass<MenuItem, Object>
{
    LODEN_OBJECT_TYPE(MenuItem)
public:
    static constexpr int TextSize = 14;
    static constexpr float BorderSize = 4;

    MenuItem();
    ~MenuItem();

    static MenuItemPtr createMenuItem(const std::string &text, const MenuPtr &menu);
    static MenuItemPtr createActionItem(const std::string &text, const ActionEventHandler &action);
    
    const std::string &getText() const;
    void setText(const std::string &newText);

    const MenuPtr &getMenu() const;
    void setMenu(const MenuPtr &newMenu);

    bool isHighlighted() const;
    void setHighlighted(bool newValue);

    void drawContentOnMenuBar(MenuBar *menuBar, Canvas *canvas);
    void drawContentOnMenu(Menu *menu, Canvas *canvas);

    void addedToMenuBar(const MenuBarPtr &menuBar);
    void addedToMenu(const MenuPtr &menu);

    const glm::vec2 &getSize() const;

    virtual void activated();

public:
    EventSocket<ActionEvent> actionEvent;

private:
    std::string text;
    MenuPtr menu;

    glm::vec2 textSize;
    glm::vec2 size;
    bool isHighlighted_;
};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_MENU_ITEM_HPP
