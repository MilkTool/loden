#include "Loden/GUI/StatusBar.hpp"
#include "Loden/Color.hpp"

namespace Loden
{
namespace GUI
{

StatusBar::StatusBar()
{
}

StatusBar::~StatusBar()
{
}

StatusBarPtr StatusBar::create()
{
    auto menuBar = std::make_shared<StatusBar>();
    return menuBar;
}

glm::vec2 StatusBar::getMinimalSize()
{
    return glm::vec2(25, 25);
}

void StatusBar::drawContentOn(Canvas *canvas)
{
    canvas->setColor(Colors::blue());
    canvas->drawFillRectangle(getLocalRectangle());
}

} // End of namespace GUI
} // End of namespace Loden