#ifndef LODEN_GUI_WIDGET_HPP
#define LODEN_GUI_WIDGET_HPP

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "Loden/Common.hpp"
#include "Loden/Rectangle.hpp"
#include "Loden/GUI/Canvas.hpp"
#include "Loden/GUI/EventSocket.hpp"
#include "Loden/GUI/Events.hpp"

namespace Loden
{
namespace GUI
{

LODEN_DECLARE_CLASS(ContainerWidget)
LODEN_DECLARE_CLASS(SystemWindow)	
LODEN_DECLARE_CLASS(Widget)

#define LODEN_WIDGET_TYPE(widgetClass, baseClass) \
public: \
	typedef baseClass BaseType; \
	std::shared_ptr<widgetClass> shared_from_this() { return std::static_pointer_cast<widgetClass> (BaseType::shared_from_this()); } \
	 
/**
 * Widget class
 */
class LODEN_CORE_EXPORT Widget : public std::enable_shared_from_this<Widget>
{
public:
	Widget();
	~Widget();
	
	virtual bool isSystemWindow() const;
	virtual SystemWindow *getSystemWindow();
	virtual glm::vec2 getAbsolutePosition() const;

	virtual bool hasKeyboardFocus() const;
	virtual bool hasMouseOver() const;
	
	void captureMouse();
	void releaseMouse();
	
	void setFocusHere();
	void setMouseOverHere();
	
	ContainerWidgetPtr getParent() const;
	void setParent(const ContainerWidgetPtr &newParent);
	
	const glm::vec2 &getPosition() const;
	void setPosition(const glm::vec2 &newPosition);
	
	const glm::vec2 &getSize() const;
	void setSize(const glm::vec2 &newSize);

	float getWidth() const;
	float getHeight() const;
	
	const glm::vec4 &getBackgroundColor();
	void setBackgroundColor(const glm::vec4 &newColor);
	
	Rectangle getRectangle() const;
	Rectangle getLocalRectangle() const;
	
public:
	virtual void drawOn(Canvas *canvas);
	virtual void drawContentOn(Canvas *canvas);

	virtual void handleKeyDown(KeyboardEvent &event);
	virtual void handleKeyUp(KeyboardEvent &event);

	virtual void handleGotFocus(FocusEvent &event);
	virtual void handleLostFocus(FocusEvent &event);
	virtual void handleMouseEnter(MouseFocusEvent &event);
	virtual void handleMouseLeave(MouseFocusEvent &event);

	virtual void handleMouseButtonDown(MouseButtonEvent &event);
	virtual void handleMouseButtonUp(MouseButtonEvent &event);
	virtual void handleMouseMotion(MouseMotionEvent &event);
	
public:
	EventSocket<KeyboardEvent> keyDownEvent;
	EventSocket<KeyboardEvent> keyUpEvent;
	
	EventSocket<MouseButtonEvent> mouseButtonDownEvent;
	EventSocket<MouseButtonEvent> mouseButtonUpEvent;
	EventSocket<MouseMotionEvent> mouseMotionEvent;
	
	EventSocket<FocusEvent> gotFocusEvent;
	EventSocket<FocusEvent> lostFocusEvent;
	EventSocket<MouseFocusEvent> mouseEnterEvent;
	EventSocket<MouseFocusEvent> mouseLeaveEvent;
	
private:
	glm::vec2 position;
	glm::vec2 size;
	glm::vec4 backgroundColor;
	ContainerWidgetWeakPtr parent;
	
	bool hasKeyboardFocus_;
	bool hasMouseOver_;
};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_WIDGET_HPP
