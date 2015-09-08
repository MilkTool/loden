#ifndef LODEN_GUI_CONTAINER_WIDGET_HPP
#define LODEN_GUI_CONTAINER_WIDGET_HPP

#include <vector>
#include "Loden/GUI/Widget.hpp"

namespace Loden
{
namespace GUI
{

LODEN_DECLARE_CLASS(ContainerWidget);

/**
 * ContainerWidget class
 */
class LODEN_CORE_EXPORT ContainerWidget: public Widget
{
	LODEN_WIDGET_TYPE(ContainerWidget, Widget);
public:
	ContainerWidget();
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
	
private:
	std::vector<WidgetPtr> children;
};

} // End of namespace GUI
} // End of namespace Loden

#endif // LODEN_GUI_CONTAINER_WIDGET_HPP
