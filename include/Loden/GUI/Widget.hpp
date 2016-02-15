#ifndef LODEN_GUI_WIDGET_HPP
#define LODEN_GUI_WIDGET_HPP

#include "Loden/Common.hpp"
#include "Loden/Rectangle.hpp"
#include "Loden/GUI/Canvas.hpp"
#include "Loden/GUI/EventSocket.hpp"
#include "Loden/GUI/Events.hpp"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace Loden
{
LODEN_DECLARE_CLASS(Engine)

namespace GUI
{

LODEN_DECLARE_CLASS(ContainerWidget)
LODEN_DECLARE_CLASS(SystemWindow)	
LODEN_DECLARE_CLASS(Widget)
LODEN_DECLARE_CLASS(Font)
LODEN_DECLARE_CLASS(FontFace)
	 
/**
 * Widget class
 */
class LODEN_CORE_EXPORT Widget : public ObjectSubclass<Widget, Object>
{
    LODEN_OBJECT_TYPE(Widget)
public:
	Widget(const SystemWindowPtr &systemWindow = nullptr);
	~Widget();
	
	virtual bool isSystemWindow() const;
    virtual EnginePtr getEngine();

    SystemWindowPtr getSystemWindow();
    void setSystemWindow(const SystemWindowPtr &newSystemWindow);

	virtual glm::vec2 getAbsolutePosition() const;

	virtual bool hasKeyboardFocus() const;
	virtual bool hasMouseOver() const;

    virtual glm::vec2 getMinimalSize();
    virtual glm::vec2 getPreferredSize();

    bool isAncestorOf(const WidgetPtr &o) const;

    FontPtr getDefaultFont();
    FontFacePtr getDefaultFontFace();

    Rectangle computeUtf16TextRectangle(const std::wstring &text, int pointSize);
    Rectangle computeUtf8TextRectangle(const std::string &text, int pointSize);

    glm::vec2 computeUtf16TextSize(const std::wstring &text, int pointSize);
    glm::vec2 computeUtf8TextSize(const std::string &text, int pointSize);

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
    void setRectangle(const Rectangle &rectangle);

	Rectangle getLocalRectangle() const;
    Rectangle getAbsoluteRectangle() const;
	
    virtual WidgetPtr getCurrentPopUpGroup();
    virtual void popUp(const WidgetPtr &popupGroup = nullptr);
    virtual void popKill(const WidgetPtr &popupGroup = nullptr);
    virtual void killAllPopUps(const WidgetPtr &popupGroup = nullptr);

public:
	virtual void drawOn(Canvas *canvas);
	virtual void drawContentOn(Canvas *canvas);

    virtual void handleAddedToParent(ParentChangedEvent &event);
    virtual void handleRemovedFromParent(ParentChangedEvent &event);

	virtual void handleKeyDown(KeyboardEvent &event);
	virtual void handleKeyUp(KeyboardEvent &event);
    virtual void handleTextInput(TextInputEvent &event);

	virtual void handleGotFocus(FocusEvent &event);
	virtual void handleLostFocus(FocusEvent &event);
	virtual void handleMouseEnter(MouseFocusEvent &event);
	virtual void handleMouseLeave(MouseFocusEvent &event);

	virtual void handleMouseButtonDown(MouseButtonEvent &event);
	virtual void handleMouseButtonUp(MouseButtonEvent &event);
	virtual void handleMouseMotion(MouseMotionEvent &event);

    virtual void handleSizeChanged(SizeChangedEvent &event);
    virtual void handlePositionChanged(PositionChangedEvent &event);

    virtual void handlePopUpsKilledEvent(PopUpsKilledEvent &event);

public:
    EventSocket<ParentChangedEvent> addedToParentEvent;
    EventSocket<ParentChangedEvent> removedFromParentEvent;

	EventSocket<KeyboardEvent> keyDownEvent;
	EventSocket<KeyboardEvent> keyUpEvent;
    EventSocket<TextInputEvent> textInputEvent;
	
	EventSocket<MouseButtonEvent> mouseButtonDownEvent;
	EventSocket<MouseButtonEvent> mouseButtonUpEvent;
	EventSocket<MouseMotionEvent> mouseMotionEvent;
	
	EventSocket<FocusEvent> gotFocusEvent;
	EventSocket<FocusEvent> lostFocusEvent;
	EventSocket<MouseFocusEvent> mouseEnterEvent;
	EventSocket<MouseFocusEvent> mouseLeaveEvent;

    EventSocket<SizeChangedEvent> sizeChangedEvent;
    EventSocket<PositionChangedEvent> positionChangedEvent;
	
    EventSocket<PopUpsKilledEvent> popUpsKilledEvent;
private:
	glm::vec2 position;
	glm::vec2 size;
	glm::vec4 backgroundColor;
    SystemWindowWeakPtr systemWindow;
	ContainerWidgetWeakPtr parent;
	
	bool hasKeyboardFocus_;
	bool hasMouseOver_;
};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_WIDGET_HPP
