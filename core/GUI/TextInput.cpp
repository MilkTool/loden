#include "Loden/GUI/TextInput.hpp"
#include "Loden/Color.hpp"

namespace Loden
{
namespace GUI
{

TextInput::TextInput(const SystemWindowPtr &systemWindow)
    : BaseType(systemWindow)
{
    fontSize = 14;
    cursor = 0;
}

TextInput::~TextInput()
{
}

TextInputPtr TextInput::create(const SystemWindowPtr &systemWindow, const std::string &text, const glm::vec2 &size, const glm::vec2 &position)
{
    auto textInput = std::make_shared<TextInput>(systemWindow);
    textInput->setText(text);
    textInput->setSize(size);
    textInput->setPosition(position);
    return textInput;
}

glm::vec2 TextInput::getMinimalSize()
{
    return glm::vec2(25, 25);
}

std::string TextInput::getText() const
{
    return std::string(textBuffer.begin(), textBuffer.end());
}

void TextInput::setText(const std::string &newText)
{
    textBuffer.clear();
    textBuffer.insert(textBuffer.end(), newText.begin(), newText.end());
    cursor = std::max(0, std::min(cursor, (int)textBuffer.size()));
}

size_t TextInput::getTextSize() const
{
    return textBuffer.size();
}

void TextInput::setFontSize(int newFontSize)
{
    fontSize = newFontSize;
}

int TextInput::getFontSize() const
{
    return fontSize;
}

void TextInput::handleKeyDown(KeyboardEvent &event)
{
    switch (event.getSymbol())
    {
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        {
            ActionEvent event;
            handleEnterAction(event);
        }
        break;
    case SDLK_ESCAPE:
        {
            ActionEvent event;
            handleCancelAction(event);
        }
        break;
    case SDLK_LEFT:
        cursor = std::min(cursor - 1, (int)textBuffer.size());
        break;
    case SDLK_RIGHT:
        cursor = std::max(cursor + 1, (int)textBuffer.size());
        break;
    case SDLK_HOME:
        cursor = 0;
        break;
    case SDLK_END:
        cursor = textBuffer.size();
        break;
    case SDLK_BACKSPACE:
        if (cursor > 0)
        {
            textBuffer.erase(textBuffer.begin() + cursor - 1);
            --cursor;
        }
        break;
    case SDLK_DELETE:
        if (cursor <= (int)textBuffer.size())
            textBuffer.erase(textBuffer.begin() + cursor);
        break;
    default:
        break;
    }

    BaseType::handleKeyDown(event);
}

void TextInput::handleKeyUp(KeyboardEvent &event)
{
    BaseType::handleKeyUp(event);
}

void TextInput::handleTextInput(TextInputEvent &event)
{
    auto &text = event.getText();
    cursor = std::min(cursor, (int)textBuffer.size());
    textBuffer.insert(textBuffer.begin() + cursor, text.begin(), text.end());
    cursor += (int)text.size();

    BaseType::handleTextInput(event);
}

void TextInput::handleMouseButtonDown(MouseButtonEvent &event)
{
    setFocusHere();
    BaseType::handleMouseButtonDown(event);
}

void TextInput::handleMouseButtonUp(MouseButtonEvent &event)
{
    BaseType::handleMouseButtonUp(event);
}

void TextInput::handleMouseMotion(MouseMotionEvent &event)
{
    BaseType::handleMouseMotion(event);
}

void TextInput::handleEnterAction(ActionEvent &event)
{
    enterActionEvent(event);
}

void TextInput::handleCancelAction(ActionEvent &event)
{
    cancelActionEvent(event);
}

void TextInput::drawContentOn(Canvas *canvas)
{
    canvas->setColor(Colors::white());
    canvas->drawFillRectangle(getLocalRectangle());

    canvas->setColor(hasKeyboardFocus() ? Colors::gray() : Colors::black());
    canvas->drawRectangle(getLocalRectangle());

    canvas->setColor(Colors::transparent()); // TODO: Implement the clip mask properly
    canvas->withClipRectangle(getLocalRectangle(), [&] {
        canvas->setColor(Colors::black());
        canvas->drawText(getText(), fontSize, glm::vec2(5, getHeight() - 5));
    });
}

} // End of namespace GUI
} // End of namespace Loden
