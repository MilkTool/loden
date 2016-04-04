#include "Loden/GUI/Menu.hpp"
#include "Loden/Color.hpp"

namespace Loden
{
namespace GUI
{

Menu::Menu(const SystemWindowPtr &systemWindow)
    : BaseType(systemWindow)
{
    activeItemIndex = -1;
}

Menu::~Menu()
{
}

MenuPtr Menu::create(const SystemWindowPtr &systemWindow)
{
    auto menu = std::make_shared<Menu> (systemWindow);
    return menu;
}

void Menu::addItem(const MenuItemPtr &item)
{
    items.push_back(item);
    item->addedToMenu(sharedFromThis());
}

void Menu::addMenu(const std::string &text, const MenuPtr &menu)
{
    addItem(MenuItem::createMenuItem(text, menu));
}

void Menu::addAction(const std::string &text, const ActionEventHandler &action)
{
    addItem(MenuItem::createActionItem(text, action));
}

void Menu::handleAddedToParent(ParentChangedEvent &event)
{
    glm::vec2 newSize;
    for (auto &item : items)
    {
        auto itemSize = item->getSize();
        newSize.x = std::max(newSize.x, itemSize.x);
        newSize.y += itemSize.y;
    }
    setSize(newSize);

    BaseType::handleAddedToParent(event);
}

void Menu::drawContentOn(Canvas *canvas)
{
    canvas->setColor(Colors::blue());
    canvas->drawFillRectangle(getLocalRectangle());

    if (!hasMouseOver())
        activeItemIndex = -1;

    glm::vec2 currentPosition;
    for (size_t i = 0; i < items.size(); ++i)
    {
        auto &item = items[i];
        item->setHighlighted(int(i) == activeItemIndex);
        canvas->withTranslation(currentPosition, [&] {
            item->drawContentOnMenu(this, canvas);
        });

        currentPosition.y += item->getSize().y;
    }

    canvas->setColor(Colors::gray());
    canvas->drawRectangle(getLocalRectangle());
}

void Menu::handleMouseButtonDown(MouseButtonEvent &event)
{
    BaseType::handleMouseButtonDown(event);

    auto item = getItemAtPosition(event.getPosition(), nullptr, nullptr);
    if (!item)
        return;

    auto &subMenu = item->getMenu();
    if (!subMenu)
    {
        killAllPopUps();
        item->activated();
    }
}

void Menu::handleMouseButtonUp(MouseButtonEvent &event)
{
    BaseType::handleMouseButtonUp(event);
}

void Menu::handleMouseMotion(MouseMotionEvent &event)
{
    BaseType::handleMouseMotion(event);

    glm::vec2 itemPosition;
    int newItemIndex;
    auto item = getItemAtPosition(event.getPosition(), &newItemIndex, &itemPosition);
    if (!item)
        return;

    if (activeItemIndex == newItemIndex)
        return;

    activeItemIndex = newItemIndex;
    auto &subMenu = item->getMenu();
    if (subMenu)
    {
        setFocusHere();
        subMenu->setPosition(getAbsolutePosition() + glm::vec2(getWidth(), itemPosition.y));
        subMenu->popUp(getCurrentPopUpGroup());
    }
}

MenuItemPtr Menu::getItemAtPosition(const glm::vec2 &position, int *itemIndex, glm::vec2 *itemPosition)
{
    auto posY = 0.0f;
    for (size_t i = 0; i < items.size(); ++i)
    {
        auto &item = items[i];
        auto itemWidth = item->getSize().y;
        if (posY <= position.y && position.y <= posY + itemWidth)
        {
            if (itemIndex)
                *itemIndex = i;
            if (itemPosition)
                *itemPosition = glm::vec2(posY, 0);
            return item;
        }
        posY += itemWidth;
    }

    if (itemIndex)
        *itemIndex = -1;
    return nullptr;
}

} // End of namespace GUI
} // End of namespace Loden
