#ifndef LODEN_GUI_BUTTON_HPP
#define LODEN_GUI_BUTTON_HPP

#include "Loden/GUI/Widget.hpp"

namespace Loden
{
namespace GUI
{

LODEN_DECLARE_CLASS(Label);

/**
* Button widget
*/
class LODEN_CORE_EXPORT Label : public Widget
{
    LODEN_WIDGET_TYPE(Label, Widget);
public:
    Label(const SystemWindowPtr &systemWindow);
    ~Label();

    static LabelPtr create(const SystemWindowPtr &systemWindow, const std::string &text, const glm::vec2 &size = glm::vec2(), const glm::vec2 &position = glm::vec2());

    virtual glm::vec2 getMinimalSize();

    const std::string &getText() const;
    void setText(const std::string &newLabel);

    int getTextSize() const;
    void setTextSize(int newTextSize);

    const glm::vec4 &getForegroundColor();
    void setForegroundColor(const glm::vec4 &newForeground);

    virtual void drawContentOn(Canvas *canvas);

private:
    std::string text;
    glm::vec4 foregroundColor;
    int textSize;
};
} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_BUTTON_HPP
