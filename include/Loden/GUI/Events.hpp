#ifndef LODEN_EVENT_HPP_
#define LODEN_EVENT_HPP_

#include "Loden/Common.hpp"
#include "SDL.h"

namespace Loden
{
namespace GUI
{
	
LODEN_DECLARE_CLASS(Widget);

/**
 * The event class.
 */	
class Event
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
 * The keyboard event.
 */
class KeyboardEvent: public Event
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
class MouseEvent: public Event
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
class MouseButtonEvent: public MouseEvent
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
class MouseMotionEvent: public MouseEvent
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
class FocusEvent: public Event
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
class MouseFocusEvent: public Event
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

} // End of namespace Loden
} // End of namespace GUI

#endif //LODEN_EVENT_HPP_