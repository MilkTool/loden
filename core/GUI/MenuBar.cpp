#include "Loden/GUI/MenuBar.hpp"
#include "Loden/Color.hpp"

namespace Loden
{
namespace GUI
{

MenuBar::MenuBar()
{
}

MenuBar::~MenuBar()
{
}

MenuBarPtr MenuBar::create()
{
    auto menuBar = std::make_shared<MenuBar>();
    return menuBar;
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