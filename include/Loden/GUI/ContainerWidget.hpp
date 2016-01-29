#ifndef LODEN_GUI_CONTAINER_WIDGET_HPP
#define LODEN_GUI_CONTAINER_WIDGET_HPP

#include "Loden/GUI/Widget.hpp"
#include <vector>

namespace Loden
{
namespace GUI
{

LODEN_DECLARE_CLASS(ContainerWidget);
LODEN_DECLARE_CLASS(Layout);

/**
 * ContainerWidget class
 */
class LODEN_CORE_EXPORT ContainerWidget: public Widget
{
	LODEN_WIDGET_TYPE(ContainerWidget, Widget);
public:
	ContainerWidget(const SystemWindowPtr &systemWindow);
	~ContainerWidget();

	size_t getNumberOfChilds();
	void addChild(const WidgetPtr& child);
	void removeChild(const WidgetPtr& child);
	
	WidgetPtr findChildAtPoint(const glm::vec2 &position);
	
	virtual void drawOn(Canvas *canvas);
	virtual void drawChildrenOn(Canvas *canvas);
	
	virtual void handleMouseButtonDown(MouseButtonEvent &event);
	virtual void handleMouseButtonUp(MouseButtonEvent &event);
	virtual void handleMouseMotion(MouseMotionEvent &event);
	
    const LayoutPtr &getLayout() const;
    void setLayout(const LayoutPtr &newLayout);

    virtual bool isAutoLayout();
    virtual void setAutoLayout(bool newAutoLayout);

    virtual void fitLayout();
    virtual void updateLayout();

private:
	std::vector<WidgetPtr> children;
    LayoutPtr layout;
    bool autoLayout;
};

} // End of namespace GUI
} // End of namespace Loden

#endif // LODEN_GUI_CONTAINER_WIDGET_HPP
