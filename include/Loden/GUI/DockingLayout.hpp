#ifndef LODEN_GUI_DOCKING_LAYOUT_HPP
#define LODEN_GUI_DOCKING_LAYOUT_HPP

#include "Loden/GUI/Layout.hpp"
#include <vector>

namespace Loden
{
namespace GUI
{

LODEN_DECLARE_CLASS(DockedElement);
LODEN_DECLARE_CLASS(DockingLayout);

/**
* Dock alignment
*/
enum class DockAlignment
{
    Bottom = 0,
    Top,
    Left,
    Right,
    Center
};

/**
 * Docked element
 */
class LODEN_CORE_EXPORT DockedElement
{
public:
    DockedElement(const WidgetPtr &widget, DockAlignment alignment, float proportion)
        : widget(widget), alignment(alignment), proportion(proportion)
    {
    }

    ~DockedElement()
    {
    }

    DockAlignment getAlignment() const
    {
        return alignment;
    }

    const WidgetPtr &getWidget() const
    {
        return widget;
    }

    float getProportion() const
    {
        return proportion;
    }

private:
    WidgetPtr widget;
    DockAlignment alignment;
    float proportion;
};

/**
 * Docking layout
 */
class LODEN_CORE_EXPORT DockingLayout : public Layout
{
public:
    DockingLayout();
    ~DockingLayout();

    DockedElementPtr addElement(const WidgetPtr &widget, DockAlignment alignment, float proportion)
    {
        auto element = std::make_shared<DockedElement>(widget, alignment, proportion);
        return addElement(element);
    }

    DockedElementPtr addElement(const DockedElementPtr &element)
    {
        elements.push_back(element);
        return element;
    }

    DockedElementPtr centerElement(const WidgetPtr &widget)
    {
        return addElement(widget, DockAlignment::Center, 1.0);
    }

    DockedElementPtr rightElement(const WidgetPtr &widget, float proportion)
    {
        return addElement(widget, DockAlignment::Right, proportion);
    }

    DockedElementPtr leftElement(const WidgetPtr &widget, float proportion)
    {
        return addElement(widget, DockAlignment::Left, proportion);
    }

    DockedElementPtr topElement(const WidgetPtr &widget, float proportion)
    {
        return addElement(widget, DockAlignment::Top, proportion);
    }

    DockedElementPtr bottomElement(const WidgetPtr &widget, float proportion)
    {
        return addElement(widget, DockAlignment::Bottom, proportion);
    }

    virtual void update(ContainerWidget *container, const Rectangle &updateRect) override;

    virtual glm::vec2 computeMinimalSize() override;
    virtual glm::vec2 computePreferredSize() override;

private:
    void applyOnElement(const DockedElementPtr &element);

    glm::vec2 extent;
    float left, right;
    float top, bottom;
    std::vector<DockedElementPtr> elements;
};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_LAYOUT_HPP
