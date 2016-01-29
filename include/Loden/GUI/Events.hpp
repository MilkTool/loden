#ifndef LODEN_EVENT_HPP_
#define LODEN_EVENT_HPP_

#include "Loden/Common.hpp"
#include "Loden/Object.hpp"
#include "Loden/GUI/EventSocket.hpp"
#include "SDL.h"

namespace Loden
{
namespace GUI
{

LODEN_DECLARE_CLASS(Widget);

#define LODEN_GUI_DECLARE_EVENT(eventName) \
    class eventName; \
    typedef EventSocket<eventName>::EventHandler eventName ## Handler;

LODEN_GUI_DECLARE_EVENT(Event);
LODEN_GUI_DECLARE_EVENT(ParentChangedEvent);
LODEN_GUI_DECLARE_EVENT(KeyboardEvent);
LODEN_GUI_DECLARE_EVENT(MouseEvent);
LODEN_GUI_DECLARE_EVENT(MouseButtonEvent);
LODEN_GUI_DECLARE_EVENT(MouseMotionEvent);
LODEN_GUI_DECLARE_EVENT(FocusEvent);
LODEN_GUI_DECLARE_EVENT(MouseFocusEvent);
LODEN_GUI_DECLARE_EVENT(ActionEvent);
LODEN_GUI_DECLARE_EVENT(SizeChangedEvent);
LODEN_GUI_DECLARE_EVENT(PositionChangedEvent);
LODEN_GUI_DECLARE_EVENT(PopUpsKilledEvent);

/**
 * The event class.
 */	
class LODEN_CORE_EXPORT Event: public Object
{
public:
	Event()
		: wasHandled_(false) {}
	~Event() {}
	
	bool wasHandled() const
	{
		return wasHandled_;
	}
	
	void setHandled(bool newHandledValue = true)
	{
		wasHandled_ = newHandledValue;
	}
	
private:
	bool wasHandled_;
};

/**
 * Parent changed event
 */
class LODEN_CORE_EXPORT ParentChangedEvent : public Event
{
public:

};

/**
 * The keyboard event.
 */
class LODEN_CORE_EXPORT KeyboardEvent: public Event
{
public:
	KeyboardEvent(SDL_Keycode symbol, bool isDown)
		: symbol(symbol), isDown_(isDown) {}
	~KeyboardEvent() {}
	
	bool isDown() const
	{
		return isDown_;
	}
	
	SDL_Keycode getSymbol() const
	{
		return symbol;
	}
	
private:
	SDL_Keycode symbol;
	bool isDown_;
};

/**
 * A mouse event.
 */
class LODEN_CORE_EXPORT MouseEvent: public Event
{
public:
	MouseEvent(glm::vec2 position)
		: position(position) {}
	~MouseEvent() {}
	
	const glm::vec2 &getPosition() const
	{
		return position;
	}
	
	void setPosition(const glm::vec2 &newPosition)
	{
		position = newPosition;
	}
	
private:
	glm::vec2 position;
};

/**
 * Mouse button event.
 */
class LODEN_CORE_EXPORT MouseButtonEvent: public MouseEvent
{
public:
	MouseButtonEvent(glm::vec2 position, int button, bool isDown)
		: MouseEvent(position), button(button), isDown_(isDown) {}

	int getButton() const
	{
		return button;
	}
	
	int isDown() const
	{
		return isDown_;
	}
	
	MouseButtonEvent translatedBy(const glm::vec2 &translation)
	{
		MouseButtonEvent copy = *this;
		copy.setPosition(getPosition() + translation);
		return copy;
	}
	
private:
	int button;
	bool isDown_;
};

/**
 * Mouse motion event.
 */
class LODEN_CORE_EXPORT MouseMotionEvent: public MouseEvent
{
public:
	MouseMotionEvent(glm::vec2 position, glm::vec2 delta)
		: MouseEvent(position), delta(delta) {}

	const glm::vec2 &getDelta() const
	{
		return delta;
	}
	
	MouseMotionEvent translatedBy(const glm::vec2 &translation)
	{
		MouseMotionEvent copy = *this;
		copy.setPosition(getPosition() + translation);
		return copy;
	}
		
private:
	glm::vec2 delta;
};

/**
 * Focus event.
 */
class LODEN_CORE_EXPORT FocusEvent: public Event
{
public:
	FocusEvent(const WidgetPtr &previousWidget, const WidgetPtr &newWidget)
		: previousWidget(previousWidget), newWidget(newWidget) {}

	const WidgetPtr &getPreviousWidget() const
	{
		return previousWidget;
	}
	
	const WidgetPtr &getNewWidget() const
	{
		return newWidget;
	}
		
private:
	WidgetPtr previousWidget;
	WidgetPtr newWidget;
};

/**
 * Mouse focus event.
 */
class LODEN_CORE_EXPORT MouseFocusEvent: public Event
{
public:
	MouseFocusEvent(const WidgetPtr &previousWidget, const WidgetPtr &newWidget)
		: previousWidget(previousWidget), newWidget(newWidget) {}

	const WidgetPtr &getPreviousWidget() const
	{
		return previousWidget;
	}
	
	const WidgetPtr &getNewWidget() const
	{
		return newWidget;
	}
		
private:
	WidgetPtr previousWidget;
	WidgetPtr newWidget;
};

/**
* Action event.
*/
class LODEN_CORE_EXPORT ActionEvent : public Event
{
public:
};

/**
* Size changed event.
*/
class LODEN_CORE_EXPORT SizeChangedEvent : public Event
{
public:
    SizeChangedEvent(const glm::vec2 &oldSize, const glm::vec2 &newSize)
        : oldSize(oldSize), newSize(newSize) {}

    const glm::vec2 &getOldSize() const
    {
        return oldSize;
    }

    const glm::vec2 &getNewSize() const
    {
        return newSize;
    }

private:
    glm::vec2 oldSize;
    glm::vec2 newSize;
};

/**
* Position changed event.
*/
class LODEN_CORE_EXPORT PositionChangedEvent : public Event
{
public:
    PositionChangedEvent(const glm::vec2 &oldPosition, const glm::vec2 &newPosition)
        : oldPosition(oldPosition), newPosition(newPosition) {}

    const glm::vec2 &getOldPosition() const
    {
        return oldPosition;
    }

    const glm::vec2 &getNewPosition() const
    {
        return newPosition;
    }

private:
    glm::vec2 oldPosition;
    glm::vec2 newPosition;
};

/**
 * PopUps killed event
 */
class LODEN_CORE_EXPORT PopUpsKilledEvent : public Event
{

};

} // End of namespace Loden
} // End of namespace GUI

#endif //LODEN_EVENT_HPP_
