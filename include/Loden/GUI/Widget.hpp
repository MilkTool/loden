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
    virtual EnginePtr getEngine();

	virtual glm::vec2 getAbsolutePosition() const;

	virtual bool hasKeyboardFocus() const;
	virtual bool hasMouseOver() const;

    virtual glm::vec2 getMinimalSize();
    virtual glm::vec2 getPreferredSize();

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

    virtual void handleSizeChanged(SizeChangedEvent &event);
    virtual void handlePositionChanged(PositionChangedEvent &event);

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

    EventSocket<SizeChangedEvent> sizeChangedEvent;
    EventSocket<PositionChangedEvent> positionChangedEvent;
	
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
