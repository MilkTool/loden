#ifndef LODEN_GUI_TEXT_INPUT_HPP
#define LODEN_GUI_TEXT_INPUT_HPP

#include "Loden/GUI/Widget.hpp"
#include <vector>

namespace Loden
{
namespace GUI
{

LODEN_DECLARE_CLASS(TextInput);

/**
* Button widget
*/
class LODEN_CORE_EXPORT TextInput : public ObjectSubclass<TextInput, Widget>
{
    LODEN_OBJECT_TYPE(TextInput);
public:
    TextInput(const SystemWindowPtr &systemWindow = nullptr);
    ~TextInput();

    static TextInputPtr create(const SystemWindowPtr &systemWindow, const std::string &text = std::string(), const glm::vec2 &size = glm::vec2(), const glm::vec2 &position = glm::vec2());

    virtual glm::vec2 getMinimalSize();

    std::string getText() const;
    void setText(const std::string &newLabel);

    size_t getTextSize() const;

    void setFontSize(int newFontSize);
    int getFontSize() const;

    virtual void handleKeyDown(KeyboardEvent &event) override;
    virtual void handleKeyUp(KeyboardEvent &event) override;
    virtual void handleTextInput(TextInputEvent &event) override;

    virtual void handleMouseButtonDown(MouseButtonEvent &event) override;
    virtual void handleMouseButtonUp(MouseButtonEvent &event) override;
    virtual void handleMouseMotion(MouseMotionEvent &event) override;

    virtual void handleEnterAction(ActionEvent &event);
    virtual void handleCancelAction(ActionEvent &event);

    virtual void drawContentOn(Canvas *canvas);

public:
    EventSocket<ActionEvent> enterActionEvent;
    EventSocket<ActionEvent> cancelActionEvent;

private:
    std::vector<char> textBuffer;
    int fontSize;
    int cursor;
};
} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_TEXT_INPUT_HPP
