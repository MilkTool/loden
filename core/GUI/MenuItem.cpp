#include "Loden/GUI/MenuItem.hpp"
#include "Loden/GUI/Menu.hpp"
#include "Loden/GUI/MenuBar.hpp"
#include "Loden/Color.hpp"

namespace Loden
{
namespace GUI
{

MenuItem::MenuItem()
{
    isHighlighted_ = false;
}

MenuItem::~MenuItem()
{
}

MenuItemPtr MenuItem::createMenuItem(const std::string &text, const MenuPtr &menu)
{
    auto item = std::make_shared<MenuItem>();
    item->setText(text);
    item->setMenu(menu);
    return item;
}

MenuItemPtr MenuItem::createActionItem(const std::string &text, const ActionEventHandler &action)
{
    auto item = std::make_shared<MenuItem>();
    item->setText(text);
    item->actionEvent += action;
    return item;
}

const std::string &MenuItem::getText() const
{
    return text;
}

void MenuItem::setText(const std::string &newText)
{
    text = newText;
}

const MenuPtr &MenuItem::getMenu() const
{
    return menu;
}

void MenuItem::setMenu(const MenuPtr &newMenu)
{
    menu = newMenu;
}

bool MenuItem::isHighlighted() const
{
    return isHighlighted_;
}

void MenuItem::setHighlighted(bool newValue)
{
    isHighlighted_ = newValue;
}

void MenuItem::drawContentOnMenuBar(MenuBar *menuBar, Canvas *canvas)
{
    auto height = menuBar->getHeight();
    if (isHighlighted())
    {
        canvas->setColor(Colors::darkGray());
        canvas->drawFillRectangle(Rectangle(glm::vec2(0, 0), glm::vec2(size.x, height)));
    }

    canvas->setColor(Colors::white());
    canvas->drawText(text, 14, glm::vec2(BorderSize, BorderSize + textSize.y + (height - BorderSize * 2 - textSize.y) / 2 ));
}

void MenuItem::drawContentOnMenu(Menu *menu, Canvas *canvas)
{
    auto width = menu->getWidth();
    if (isHighlighted())
    {
        canvas->setColor(Colors::darkGray());
        canvas->drawFillRectangle(Rectangle(glm::vec2(0, 0), glm::vec2(width, size.y)));
    }

    canvas->setColor(Colors::white());
    canvas->drawText(text, 14, glm::vec2(BorderSize, BorderSize + textSize.y));
}

void MenuItem::addedToMenuBar(const MenuBarPtr &menuBar)
{
    textSize = menuBar->computeUtf8TextSize(text, 14);
    size = glm::vec2(textSize.x + BorderSize * 2, textSize.y);
}

void MenuItem::addedToMenu(const MenuPtr &menu)
{
    textSize = menu->computeUtf8TextSize(text, 14);
    size = glm::vec2(textSize.x + BorderSize * 2, textSize.y + BorderSize * 2);
}

const glm::vec2 &MenuItem::getSize() const
{
    return size;
}

void MenuItem::activated()
{
    ActionEvent event;
    actionEvent(event);
}

} // End of namespace GUI
} // End of namespace Loden
