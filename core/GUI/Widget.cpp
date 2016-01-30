#include "Loden/GUI/Widget.hpp"
#include "Loden/GUI/ContainerWidget.hpp"
#include "Loden/GUI/SystemWindow.hpp"
#include "Loden/GUI/FontManager.hpp"
#include "Loden/Engine.hpp"
#include "Loden/Printing.hpp"

namespace Loden
{
namespace GUI
{

Widget::Widget(const SystemWindowPtr &systemWindow)
    : systemWindow(systemWindow)
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

glm::vec2 Widget::getMinimalSize()
{
    return glm::vec2(20, 20);
}

glm::vec2 Widget::getPreferredSize()
{
    return getMinimalSize();
}

FontPtr Widget::getDefaultFont()
{
    auto engine = getEngine();
    if (!engine)
        return nullptr;

    return engine->getFontManager()->getDefaultFont();
}

FontFacePtr Widget::getDefaultFontFace()
{
    auto font = getDefaultFont();
    if (!font)
        return nullptr;

    return font->getDefaultFace();
}

Rectangle Widget::computeUtf16TextRectangle(const std::wstring &text, int pointSize)
{
    auto fontFace = getDefaultFontFace();
    if (!fontFace)
        return Rectangle();

    return fontFace->computeUtf16TextRectangle(text, pointSize);
}

Rectangle Widget::computeUtf8TextRectangle(const std::string &text, int pointSize)
{
    auto fontFace = getDefaultFontFace();
    if (!fontFace)
        return Rectangle();

    return fontFace->computeUtf8TextRectangle(text, pointSize);
}

glm::vec2 Widget::computeUtf16TextSize(const std::wstring &text, int pointSize)
{
    return computeUtf16TextRectangle(text, pointSize).getSize();
}

glm::vec2 Widget::computeUtf8TextSize(const std::string &text, int pointSize)
{
    return computeUtf8TextRectangle(text, pointSize).getSize();
}

SystemWindowPtr Widget::getSystemWindow()
{
	return systemWindow.lock();
}

void Widget::setSystemWindow(const SystemWindowPtr &newSystemWindow)
{
    systemWindow = newSystemWindow;
}

EnginePtr Widget::getEngine()
{
    auto systemWindow = getSystemWindow();
    if (systemWindow)
        return systemWindow->getEngine();
    return nullptr;
}

bool Widget::isAncestorOf(const WidgetPtr &o) const
{
    auto currentParent = o->getParent();
    for (; currentParent; currentParent = currentParent->getParent())
    {
        if (currentParent.get() == this)
            return true;
    }

    return false;
}

ContainerWidgetPtr Widget::getParent() const
{
	return parent.lock();
}

void Widget::setParent(const ContainerWidgetPtr &newParent)
{
    if (parent.lock())
    {
        ParentChangedEvent event;
        handleRemovedFromParent(event);
    }

	parent = newParent;

    if (newParent)
    {
        ParentChangedEvent event;
        handleAddedToParent(event);
    }
}

const glm::vec2 &Widget::getPosition() const
{
	return position;
}

void Widget::setPosition(const glm::vec2 &newPosition)
{
    PositionChangedEvent event(position, newPosition);
	position = newPosition;
    handlePositionChanged(event);
}

const glm::vec2 &Widget::getSize() const
{
	return size;
}

void Widget::setSize(const glm::vec2 &newSize)
{
    SizeChangedEvent event(size, newSize);
	size = newSize;
    handleSizeChanged(event);
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

void Widget::setRectangle(const Rectangle &rectangle)
{
    setPosition(rectangle.min);
    setSize(rectangle.getSize());
}

Rectangle Widget::getLocalRectangle() const
{
	return Rectangle(glm::vec2(), size);
}

Rectangle Widget::getAbsoluteRectangle() const
{
    return Rectangle(getAbsolutePosition(), position + size);
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
}

void Widget::handleKeyDown(KeyboardEvent &event)
{
	keyDownEvent(event);
}

void Widget::handleKeyUp(KeyboardEvent &event)
{
	keyUpEvent(event);
}

void Widget::handleAddedToParent(ParentChangedEvent &event)
{
    addedToParentEvent(event);
}

void Widget::handleRemovedFromParent(ParentChangedEvent &event)
{
    removedFromParentEvent(event);
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

void Widget::handleSizeChanged(SizeChangedEvent &event)
{
    sizeChangedEvent(event);
}

void Widget::handlePositionChanged(PositionChangedEvent &event)
{
    positionChangedEvent(event);
}

void Widget::handlePopUpsKilledEvent(PopUpsKilledEvent &event)
{
    popUpsKilledEvent(event);
}

WidgetPtr Widget::getCurrentPopUpGroup()
{
    return getSystemWindow()->getCurrentPopUpGroup();
}

void Widget::popUp(const WidgetPtr &popupGroup)
{
    getSystemWindow()->activatePopUp(shared_from_this(), popupGroup);
}

void Widget::popKill(const WidgetPtr &popupGroup)
{
    getSystemWindow()->killPopUp(shared_from_this(), popupGroup);
}

void Widget::killAllPopUps(const WidgetPtr &popupGroup)
{
    getSystemWindow()->killAllPopUps(popupGroup);
}

} // End of namespace GUI
} // End of namespace Loden
