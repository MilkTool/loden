#include "Loden/GUI/Layout.hpp"
#include "Loden/GUI/ContainerWidget.hpp"

namespace Loden
{
namespace GUI
{

// The layout
Layout::Layout()
{
}

Layout::~Layout()
{
}

void Layout::fit(ContainerWidget *container)
{
    container->setSize(computePreferredSize());
    fitChildren(container);
    update(container, container->getLocalRectangle());
}

void Layout::fitChildren(ContainerWidget *container)
{
}

// Box layout element
glm::vec2 CellLayoutElement::getFullBorderSize()
{
    glm::vec2 size(0, 0);
    if (((int)flags & (int)CellLayoutFlags::LeftBorder) != 0)
        size.x += borderSize;
    if (((int)flags & (int)CellLayoutFlags::RightBorder) != 0)
        size.x += borderSize;
    if (((int)flags & (int)CellLayoutFlags::TopBorder) != 0)
        size.y += borderSize;
    if (((int)flags & (int)CellLayoutFlags::BottomBorder) != 0)
        size.y += borderSize;

    return size;
}

glm::vec2 CellLayoutElement::computeMinimalSize()
{
    glm::vec2 size = getFullBorderSize();
    if (widget)
        size += widget->getMinimalSize();
    else if (layout)
        size += layout->computeMinimalSize();
    return size;
}

glm::vec2 CellLayoutElement::computePreferredSize()
{
    glm::vec2 size = getFullBorderSize();
    if (widget)
        size += widget->getPreferredSize();
    else if (layout)
        size += layout->computePreferredSize();
    return size;
}

void CellLayoutElement::update(ContainerWidget *container, Rectangle updateRect)
{
    if (!widget && !layout)
        return;

    if (((int)flags & (int)CellLayoutFlags::LeftBorder) != 0)
        updateRect.min.x += borderSize;
    if (((int)flags & (int)CellLayoutFlags::RightBorder) != 0)
        updateRect.max.x -= borderSize;
    if (((int)flags & (int)CellLayoutFlags::TopBorder) != 0)
        updateRect.min.y += borderSize;
    if (((int)flags & (int)CellLayoutFlags::BottomBorder) != 0)
        updateRect.max.y -= borderSize;

    if (widget)
    {
        if (((int)flags & (int)CellLayoutFlags::Expand) != 0)
        {
            widget->setRectangle(updateRect);
        }
        else
        {
            auto availableSize = updateRect.getSize();
            widget->setSize(widget->getPreferredSize());

            auto size = widget->getSize();
            glm::vec2 position = updateRect.min;

            if (((int)flags & (int)CellLayoutFlags::AlignLeft) != 0)
                position.x = updateRect.min.x;
            if (((int)flags & (int)CellLayoutFlags::AlignRight) != 0)
                position.x = updateRect.max.x - size.x;
            if (((int)flags & (int)CellLayoutFlags::AlignCenterHorizontally) != 0)
                position.x = floor(updateRect.min.x + (availableSize.x - size.x) / 2);

            if (((int)flags & (int)CellLayoutFlags::AlignTop) != 0)
                position.y = updateRect.min.y;
            if (((int)flags & (int)CellLayoutFlags::AlignBottom) != 0)
                position.y = updateRect.max.y - size.y;
            if (((int)flags & (int)CellLayoutFlags::AlignCenterVertically) != 0)
                position.y = floor(updateRect.min.y + (availableSize.y - size.y) / 2);

            widget->setPosition(position);
        }
        
    }
    else if (layout)
        layout->update(container, updateRect);
}

// The box layout
CellLayout::CellLayout()
{
}

CellLayout::~CellLayout()
{
}


// Fill layout
FillLayout::FillLayout()
{
}

FillLayout::~FillLayout()
{
}

const WidgetPtr &FillLayout::getWidget() const
{
    return widget;
}

void FillLayout::setWidget(const WidgetPtr &newWidget)
{
    widget = newWidget;
}

void FillLayout::update(ContainerWidget *container, const Rectangle &updateRect)
{
    widget->setRectangle(updateRect);
}

glm::vec2 FillLayout::computeMinimalSize()
{
    return widget->getMinimalSize();
}

glm::vec2 FillLayout::computePreferredSize()
{
    return widget->getPreferredSize();
}

// Vertical box layout
VerticalBoxLayout::VerticalBoxLayout()
{
}

VerticalBoxLayout::~VerticalBoxLayout()
{
}

void VerticalBoxLayout::update(ContainerWidget *container, const Rectangle &updateRect)
{
    auto totalProportion = 0;
    glm::vec2 usedSize(0, 0);
    for (auto &element : elements)
    {
        auto elementProportion = element->getProportion();
        if (elementProportion == 0)
        {
            auto size = element->computePreferredSize();
            usedSize.x = std::max(size.x, usedSize.x);
            usedSize.y += size.y;
        }
        else
        {
            totalProportion += elementProportion;
        }
    }

    glm::vec2 rectSize = updateRect.getSize();
    float availableSize = rectSize.y - usedSize.y;
    if (availableSize < 0)
        availableSize = 0;

    float currentPosition = updateRect.min.y;
    for (auto &element : elements)
    {
        auto elementProportion = element->getProportion();
        float height;
        if (elementProportion == 0)
            height = element->computePreferredSize().y;
        else
            height = availableSize*elementProportion / totalProportion;

        auto position = glm::vec2(updateRect.min.x, currentPosition);
        auto size = glm::vec2(rectSize.x, height);
        element->update(container, Rectangle(position, position + size));
        currentPosition += height;
    }
}

glm::vec2 VerticalBoxLayout::computeMinimalSize()
{
    glm::vec2 minimalExtent;
    for (auto &element : elements)
    {
        auto elementSize = element->computeMinimalSize();
        minimalExtent.x = std::max(minimalExtent.x, elementSize.x);
        minimalExtent.y += elementSize.y;
    }

    return minimalExtent;
}

glm::vec2 VerticalBoxLayout::computePreferredSize()
{
    glm::vec2 preferredExtent;
    for (auto &element : elements)
    {
        auto elementSize = element->computePreferredSize();
        preferredExtent.x = std::max(preferredExtent.x, elementSize.x);
        preferredExtent.y += elementSize.y;
    }

    return preferredExtent;
}

// Horizontal box layout
HorizontalBoxLayout::HorizontalBoxLayout()
{
}

HorizontalBoxLayout::~HorizontalBoxLayout()
{
}

glm::vec2 HorizontalBoxLayout::computeMinimalSize()
{
    glm::vec2 minimalExtent;
    for (auto &element : elements)
    {
        auto elementSize = element->computeMinimalSize();
        minimalExtent.x += elementSize.x;
        minimalExtent.y = std::max(minimalExtent.y, elementSize.y);
    }

    return minimalExtent;
}

glm::vec2 HorizontalBoxLayout::computePreferredSize()
{
    glm::vec2 preferredExtent;
    for (auto &element : elements)
    {
        auto elementSize = element->computePreferredSize();
        preferredExtent.x += elementSize.x;
        preferredExtent.y = std::max(preferredExtent.y, elementSize.y);
    }

    return preferredExtent;
}

void HorizontalBoxLayout::update(ContainerWidget *container, const Rectangle &updateRect)
{
    auto totalProportion = 0;
    glm::vec2 usedSize(0, 0);
    for (auto &element : elements)
    {
        auto elementProportion = element->getProportion();
        if (elementProportion == 0)
        {
            auto size = element->computePreferredSize();
            usedSize.x += size.x;
            usedSize.y = std::max(size.y, usedSize.y);
        }
        else
        {
            totalProportion += elementProportion;
        }
    }

    glm::vec2 rectSize = updateRect.getSize();
    float availableSize = rectSize.x - usedSize.x;
    if (availableSize < 0)
        availableSize = 0;

    float currentPosition = updateRect.min.x;
    for (auto &element : elements)
    {
        auto elementProportion = element->getProportion();
        float width;
        if (elementProportion == 0)
            width = element->computePreferredSize().x;
        else
            width = availableSize*elementProportion / totalProportion;

        auto position = glm::vec2(currentPosition, updateRect.min.y);
        auto size = glm::vec2(width, rectSize.y);
        element->update(container, Rectangle(position, position + size));
        currentPosition += width;
    }
}

} // End of namespace GUI
} // End of namespace Loden
