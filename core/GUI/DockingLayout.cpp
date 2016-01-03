#include "Loden/GUI/DockingLayout.hpp"
#include "Loden/GUI/ContainerWidget.hpp"

namespace Loden
{
namespace GUI
{

DockingLayout::DockingLayout()
{
}

DockingLayout::~DockingLayout()
{
}

void DockingLayout::update(ContainerWidget *container, const Rectangle &updateRect)
{
    // Sort the elements according to the alignment.
    std::sort(elements.begin(), elements.end(), [](const DockedElementPtr &a, const DockedElementPtr &b) {
        return a->getAlignment() < b->getAlignment();
    });

    // Reset the sizes.
    extent = updateRect.getSize();
    left = updateRect.min.x; right = updateRect.max.x;
    bottom = updateRect.min.y; top = updateRect.max.y;

    // Apply to the elements
    for (auto &element : elements)
        applyOnElement(element);
}

glm::vec2 DockingLayout::computeMinimalSize()
{
    glm::vec2 minimalExtent(0, 0);
    for (auto &element : elements)
        minimalExtent += element->getWidget()->getMinimalSize();
    return minimalExtent;
}

glm::vec2 DockingLayout::computePreferredSize()
{
    glm::vec2 preferredExtent(0, 0);
    for (auto &element : elements)
        preferredExtent += element->getWidget()->getPreferredSize();
    return preferredExtent;
}

void DockingLayout::applyOnElement(const DockedElementPtr &element)
{
    // Compute the element extent.
    glm::vec2 elementExtent;
    switch (element->getAlignment())
    {
    case DockAlignment::Bottom:
    case DockAlignment::Top:
        elementExtent = glm::vec2(extent.x, extent.y*element->getProportion());
        extent.y -= elementExtent.y;
        break;
    case DockAlignment::Left:
    case DockAlignment::Right:
        elementExtent = glm::vec2(extent.x*element->getProportion(), extent.y);
        extent.x -= elementExtent.x;
        break;
    case DockAlignment::Center:
        elementExtent = extent*element->getProportion();
        extent -= elementExtent;
        break;
    }

    // Compute the element position.
    glm::vec2 elementPosition;
    switch (element->getAlignment())
    {
    case DockAlignment::Bottom:
        elementPosition = glm::vec2(left, bottom);
        bottom += elementExtent.y;
        break;
    case DockAlignment::Top:
        top -= elementExtent.y;
        elementPosition = glm::vec2(left, top);
        break;
    case DockAlignment::Left:
        elementPosition = glm::vec2(left, bottom);
        left += elementExtent.x;
        break;
    case DockAlignment::Right:
        right -= elementExtent.x;
        elementPosition = glm::vec2(right, bottom);
        break;
    case DockAlignment::Center:
        elementPosition = glm::vec2(left, bottom);
        left += elementExtent.x;
        bottom += elementExtent.y;
        break;
    }

    // Set the widget position and size
    const auto &widget = element->getWidget();
    widget->setSize(elementExtent);
    widget->setPosition(elementPosition);
}

} // End of namespace GUI
} // End of namespace Loden
