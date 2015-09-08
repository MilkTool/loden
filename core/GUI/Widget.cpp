#include "Loden/GUI/Widget.hpp"
#include "Loden/GUI/ContainerWidget.hpp"
#include "Loden/GUI/SystemWindow.hpp"
#include "Loden/Printing.hpp"

namespace Loden
{
namespace GUI
{

Widget::Widget()
{
	hasKeyboardFocus_ = false;
	hasMouseOver_ = false;
}

Widget::~Widget()
{
}

bool Widget::isSystemWindow() const
{
	return false;
}

bool Widget::hasKeyboardFocus() const
{
	return hasKeyboardFocus_;
}

bool Widget::hasMouseOver() const
{
	return hasMouseOver_;
}

void Widget::captureMouse()
{
	getSystemWindow()->setMouseCaptureWidget(shared_from_this());
}

void Widget::releaseMouse()
{
	getSystemWindow()->setMouseCaptureWidget(WidgetPtr());
}

void Widget::setFocusHere()
{
	getSystemWindow()->setKeyboardFocusWidget(shared_from_this());
}

void Widget::setMouseOverHere()
{
	getSystemWindow()->setMouseFocusWidget(shared_from_this());
}

glm::vec2 Widget::getAbsolutePosition() const
{
	auto theParent = getParent();
	if(theParent)
		return theParent->getAbsolutePosition() + getPosition();
	return getPosition();
}

SystemWindow *Widget::getSystemWindow()
{
	auto theParent = getParent();
	if(theParent)
		return theParent->getSystemWindow();
	return nullptr;
}

ContainerWidgetPtr Widget::getParent() const
{
	return parent.lock();
}

void Widget::setParent(const ContainerWidgetPtr &newParent)
{
	parent = newParent;
}

const glm::vec2 &Widget::getPosition() const
{
	return position;
}

void Widget::setPosition(const glm::vec2 &newPosition)
{
	position = newPosition;
}

const glm::vec2 &Widget::getSize() const
{
	return size;
}

void Widget::setSize(const glm::vec2 &newSize)
{
	size = newSize;
}

float Widget::getWidth() const
{
	return size.x;
}

float Widget::getHeight() const
{
	return size.y;
}

const glm::vec4 &Widget::getBackgroundColor()
{
	return backgroundColor;
}

void Widget::setBackgroundColor(const glm::vec4 &newColor)
{
	backgroundColor = newColor;
}

Rectangle Widget::getRectangle() const
{
	return Rectangle(position, position + size);
}

Rectangle Widget::getLocalRectangle() const
{
	return Rectangle(glm::vec2(), size);
}

void Widget::drawOn(Canvas *canvas)
{
	canvas->withTranslation(getPosition(), [&] {
		// Draw the content
		drawContentOn(canvas);
	});
}

void Widget::drawContentOn(Canvas *canvas)
{
	canvas->setColor(getBackgroundColor());
	canvas->drawFillRectangle(getLocalRectangle());
}

void Widget::handleKeyDown(KeyboardEvent &event)
{
	keyDownEvent(event);
}

void Widget::handleKeyUp(KeyboardEvent &event)
{
	keyUpEvent(event);
}

void Widget::handleGotFocus(FocusEvent &event)
{
	hasKeyboardFocus_ = true;
	gotFocusEvent(event);
}

void Widget::handleLostFocus(FocusEvent &event)
{
	hasKeyboardFocus_ = false;
	lostFocusEvent(event);
}

void Widget::handleMouseEnter(MouseFocusEvent &event)
{
	hasMouseOver_ = true;
	mouseEnterEvent(event);
}

void Widget::handleMouseLeave(MouseFocusEvent &event)
{
	hasMouseOver_ = false ;
	mouseLeaveEvent(event);
}

void Widget::handleMouseButtonDown(MouseButtonEvent &event)
{
	setMouseOverHere();
	mouseButtonDownEvent(event);
}

void Widget::handleMouseButtonUp(MouseButtonEvent &event)
{
	setMouseOverHere();
	mouseButtonUpEvent(event);
}

void Widget::handleMouseMotion(MouseMotionEvent &event)
{
	setMouseOverHere();
	mouseMotionEvent(event);
}

} // End of namespace GUI
} // End of namespace Loden
