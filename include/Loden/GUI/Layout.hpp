#ifndef LODEN_GUI_LAYOUT_HPP
#define LODEN_GUI_LAYOUT_HPP

#include "Loden/GUI/Widget.hpp"
#include <vector>

namespace Loden
{
namespace GUI
{

LODEN_DECLARE_CLASS(ContainerWidget);
LODEN_DECLARE_CLASS(Layout);
LODEN_DECLARE_CLASS(CellLayoutElement);
LODEN_DECLARE_CLASS(BoxLayout);
LODEN_DECLARE_CLASS(VerticalBoxLayout);
LODEN_DECLARE_CLASS(HorizontalBoxLayout);

/**
* Layout
*/
class LODEN_CORE_EXPORT Layout: public ObjectAbstractSubclass<Layout, Object>
{
    LODEN_OBJECT_TYPE(Layout)
public:
    Layout();
    ~Layout();

    virtual void fit(ContainerWidget *container);
    virtual void fitChildren(ContainerWidget *container);
    virtual void update(ContainerWidget *container, const Rectangle &updateRect) = 0;

    virtual glm::vec2 computeMinimalSize() = 0;
    virtual glm::vec2 computePreferredSize() = 0;

private:

};

/**
* Fill layout
*/
class LODEN_CORE_EXPORT FillLayout : public ObjectSubclass<FillLayout, Layout>
{
    LODEN_OBJECT_TYPE(FillLayout)
public:
    FillLayout();
    ~FillLayout();

    const WidgetPtr &getWidget() const;
    void setWidget(const WidgetPtr &newWidget);

    virtual void update(ContainerWidget *container, const Rectangle &updateRect) override;

    virtual glm::vec2 computeMinimalSize() override;
    virtual glm::vec2 computePreferredSize() override;


private:
    WidgetPtr widget;
};

/**
 * Box layout flags
 */
enum class CellLayoutFlags
{
    None = 0,
    TopBorder = 1,
    BottomBorder = 1<<1,
    LeftBorder = 1<<2,
    RightBorder = 1<<3,
    Expand = 1<<4,
    Shaped = 1<<5,

    AlignLeft = 1<<6,
    AlignRight = 1 << 7,
    AlignTop = 1 << 8,
    AlignBottom = 1 << 9,
    AlignCenterHorizontally = 1 << 10,
    AlignCenterVertically = 1 << 11,
    AlignCenter = AlignCenterHorizontally | AlignCenterVertically,

    AllBorders = TopBorder | BottomBorder | LeftBorder | RightBorder,
    ExpandShaped = Expand | Shaped,
    Default = None,
};

/**
 * Box layout element
 */
class LODEN_CORE_EXPORT CellLayoutElement
{
public:
    CellLayoutElement(const WidgetPtr &widget, int proportion, float borderSize, CellLayoutFlags flags = CellLayoutFlags::None)
        : widget(widget), proportion(proportion), borderSize(borderSize), flags(flags)
    {
    }

    CellLayoutElement(const LayoutPtr &layout, int proportion, float borderSize, CellLayoutFlags flags = CellLayoutFlags::None)
        : layout(layout), proportion(proportion), borderSize(borderSize), flags(flags)
    {
    }

    CellLayoutElement(int proportion, float borderSize, CellLayoutFlags flags = CellLayoutFlags::None)
        : proportion(proportion), borderSize(borderSize), flags(flags)
    {
    }

    const WidgetPtr &getWidget() const
    {
        return widget;
    }

    int getProportion() const
    {
        return proportion;
    }

    float getBorderSize() const
    {
        return borderSize;
    }

    CellLayoutFlags getFlags() const
    {
        return flags;
    }

    void update(ContainerWidget *container, Rectangle updateRect);

    glm::vec2 getFullBorderSize();
    glm::vec2 computeMinimalSize();
    glm::vec2 computePreferredSize();

private:
    WidgetPtr widget;
    LayoutPtr layout;
    int proportion;
    float borderSize;
    CellLayoutFlags flags;
};

/**
* Cell based layout
*/
class LODEN_CORE_EXPORT CellLayout : public ObjectAbstractSubclass<CellLayout, Layout>
{
    LODEN_OBJECT_TYPE(CellLayout)
public:
    CellLayout();
    ~CellLayout();

    CellLayout &addElement(const CellLayoutElementPtr &element)
    {
        elements.push_back(element);
        return *this;
    }

    CellLayout &addWidget(const WidgetPtr &widget, int proportion = 0, float borderSize = 0, CellLayoutFlags flags = CellLayoutFlags::Default)
    {
        return addElement(std::make_shared<CellLayoutElement> (widget, proportion, borderSize, flags));
    }

    CellLayout &addSpace(int proportion = 0, float borderSize = 0, CellLayoutFlags flags = CellLayoutFlags::Default)
    {
        return addElement(std::make_shared<CellLayoutElement>(proportion, borderSize, flags));
    }

    CellLayout &addLayout(const LayoutPtr &layout, int proportion = 0, float borderSize = 0, CellLayoutFlags flags = CellLayoutFlags::Default)
    {
        return addElement(std::make_shared<CellLayoutElement>(layout, proportion, borderSize, flags));
    }

protected:
    std::vector<CellLayoutElementPtr> elements;
};

/**
* Vertical box layout
*/
class LODEN_CORE_EXPORT VerticalBoxLayout : public ObjectSubclass<VerticalBoxLayout, CellLayout>
{
    LODEN_OBJECT_TYPE(VerticalBoxLayout)
public:
    VerticalBoxLayout();
    ~VerticalBoxLayout();

    virtual void update(ContainerWidget *container, const Rectangle &updateRect) override;

    virtual glm::vec2 computeMinimalSize() override;
    virtual glm::vec2 computePreferredSize() override;

private:

};

/**
* Horizontal box layout
*/
class LODEN_CORE_EXPORT HorizontalBoxLayout : public ObjectSubclass<HorizontalBoxLayout, CellLayout>
{
    LODEN_OBJECT_TYPE(HorizontalBoxLayout)
public:
    HorizontalBoxLayout();
    ~HorizontalBoxLayout();

    virtual void update(ContainerWidget *container, const Rectangle &updateRect) override;

    virtual glm::vec2 computeMinimalSize() override;
    virtual glm::vec2 computePreferredSize() override;

private:

};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_LAYOUT_HPP
