#include "Loden/GUI/MenuBar.hpp"
#include "Loden/Color.hpp"

namespace Loden
{
namespace GUI
{

MenuBar::MenuBar(const SystemWindowPtr &systemWindow)
    : BaseType(systemWindow)
{
    activated = false;
    activeItemIndex = -1;
}

MenuBar::~MenuBar()
{
}

MenuBarPtr MenuBar::create(const SystemWindowPtr &systemWindow)
{
    auto menuBar = std::make_shared<MenuBar>(systemWindow);
    return menuBar;
}

void MenuBar::addItem(const MenuItemPtr &item)
{
    items.push_back(item);
    item->addedToMenuBar(sharedFromThis());
}

void MenuBar::addMenu(const std::string &text, const MenuPtr &menu)
{
    addItem(MenuItem::createMenuItem(text, menu));
}

void MenuBar::addAction(const std::string &text, const ActionEventHandler &action)
{
    addItem(MenuItem::createActionItem(text, action));
}

glm::vec2 MenuBar::getMinimalSize()
{
    return glm::vec2(25, 25);
}

void MenuBar::drawContentOn(Canvas *canvas)
{
    canvas->setColor(Colors::blue());
    canvas->drawFillRectangle(getLocalRectangle());

    glm::vec2 currentPosition;
    for (size_t i = 0; i < items.size(); ++i)
    {
        auto &item = items[i];
        item->setHighlighted(activated && int(i) == activeItemIndex);
        canvas->withTranslation(currentPosition, [&] {
            item->drawContentOnMenuBar(this, canvas);
        });

        currentPosition.x += item->getSize().x;
    }
}

MenuItemPtr MenuBar::getItemAtPosition(const glm::vec2 &position, int *itemIndex, glm::vec2 *itemPosition)
{
    auto posX = 0.0f;
    for (size_t i = 0; i < items.size(); ++i)
    {
        auto &item = items[i];
        auto itemWidth = item->getSize().x;
        if (posX <= position.x && position.x <= posX + itemWidth)
        {
            if (itemIndex)
                *itemIndex = i;
            if (itemPosition)
                *itemPosition = glm::vec2(posX, 0);
            return item;
        }
        posX += itemWidth;
    }

    if(itemIndex)
        *itemIndex = -1;
    return nullptr;
}

void MenuBar::handleMouseButtonDown(MouseButtonEvent &event)
{
    BaseType::handleMouseButtonDown(event);

    // Deactivate the menu if it was active
    if (activated)
    {
        killAllPopUps();
        auto item = getItemAtPosition(event.getPosition(), nullptr, nullptr);
        if (item && !item->getMenu())
            item->activated();
        return;
    }

    activateMenuAtPosition(event.getPosition());
}

void MenuBar::handleMouseButtonUp(MouseButtonEvent &event)
{
    BaseType::handleMouseButtonUp(event);
}

void MenuBar::handleMouseMotion(MouseMotionEvent &event)
{
    BaseType::handleMouseMotion(event);

    if (activated)
        activateMenuAtPosition(event.getPosition());
}

void MenuBar::activateMenuAtPosition(const glm::vec2 &position)
{
    glm::vec2 itemPosition;
    int newItemIndex;
    auto item = getItemAtPosition(position, &newItemIndex, &itemPosition);
    if (!item)
        return;

    if (activated && activeItemIndex == newItemIndex)
        return;

    killAllPopUps();
    activated = true;
    activeItemIndex = newItemIndex;
    auto &subMenu = item->getMenu();
    if (subMenu)
    {
        subMenu->setPosition(getAbsolutePosition() + glm::vec2(itemPosition.x, getHeight()));
        subMenu->popUp(sharedFromThis());
    }
    else
    {
        item->activated();
    }
}

void MenuBar::handlePopUpsKilledEvent(PopUpsKilledEvent &event)
{
    activated = false;
    activeItemIndex = -1;
    BaseType::handlePopUpsKilledEvent(event);
}


} // End of namespace GUI
} // End of namespace Loden