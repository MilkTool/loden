#include "Loden/GUI/Label.hpp"
#include "Loden/Color.hpp"

namespace Loden
{
namespace GUI
{

Label::Label(const SystemWindowPtr &systemWindow)
    : Widget(systemWindow)
{
    setForegroundColor(Colors::white());
    setBackgroundColor(Colors::transparent());
    setTextSize(12);
}

Label::~Label()
{
}

LabelPtr Label::create(const SystemWindowPtr &systemWindow, const std::string &text, const glm::vec2 &size, const glm::vec2 &position)
{
    auto label = std::make_shared<Label>(systemWindow);
    label->setText(text);
    label->setPosition(position);
    label->setSize(size);
    return label;
}

glm::vec2 Label::getMinimalSize()
{
    return computeUtf8TextSize(text, textSize);
}

const std::string &Label::getText() const
{
    return text;
}

void Label::setText(const std::string &newText)
{
    text = newText;
}

int Label::getTextSize() const
{
    return textSize;
}

void Label::setTextSize(int newTextSize)
{
    textSize = newTextSize;
}

const glm::vec4 &Label::getForegroundColor()
{
    return foregroundColor;
}

void Label::setForegroundColor(const glm::vec4 &newForeground)
{
    foregroundColor = newForeground;
}

void Label::drawContentOn(Canvas *canvas)
{
    auto background = getBackgroundColor();
    if (!isFullyTransparentColor(background))
    {
        canvas->setColor(background);
        canvas->drawFillRectangle(getLocalRectangle());
    }

    canvas->setColor(getForegroundColor());
    canvas->drawText(text, textSize, glm::vec2(5, getHeight() - 5));
}

} // End of namespace GUI
} // End of namespace Loden
