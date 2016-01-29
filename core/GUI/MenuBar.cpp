#include "Loden/GUI/MenuBar.hpp"
#include "Loden/Color.hpp"

namespace Loden
{
namespace GUI
{

MenuBar::MenuBar(const SystemWindowPtr &systemWindow)
    : Widget(systemWindow)
{
}

MenuBar::~MenuBar()
{
}

MenuBarPtr MenuBar::create(const SystemWindowPtr &systemWindow)
{
    auto menuBar = std::make_shared<MenuBar>(systemWindow);
    return menuBar;
}

void MenuBar::addMenu(const std::string &text, const MenuPtr &menu)
{
    items.push_back(Item(text, menu));
}

glm::vec2 MenuBar::getMinimalSize()
{
    return glm::vec2(25, 25);
}

void MenuBar::drawContentOn(Canvas *canvas)
{
    canvas->setColor(Colors::blue());
    canvas->drawFillRectangle(getLocalRectangle());
}

} // End of namespace GUI
} // End of namespace Loden