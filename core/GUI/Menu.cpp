#include "Loden/GUI/Menu.hpp"

namespace Loden
{
namespace GUI
{

Menu::Menu(const SystemWindowPtr &systemWindow)
    : Widget(systemWindow)
{
}

Menu::~Menu()
{
}

MenuPtr Menu::create(const SystemWindowPtr &systemWindow)
{
    auto menu = std::make_shared<Menu> (systemWindow);
    return menu;
}

void Menu::addItem(const std::string &text, const ActionEventHandler &action)
{
}

glm::vec2 Menu::getMinimalSize()
{
    return glm::vec2(10, 10);
}

void Menu::drawContentOn(Canvas *canvas)
{
}

} // End of namespace GUI
} // End of namespace Loden

