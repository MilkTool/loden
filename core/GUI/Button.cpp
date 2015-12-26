#include "Loden/GUI/Button.hpp"
#include "Loden/Color.hpp"

namespace Loden
{
namespace GUI
{

Button::Button()
{
	isButtonDown_ = false;
}

Button::~Button()
{
}

ButtonPtr Button::create(const std::string &label, const glm::vec2 &size, const glm::vec2 &position)
{
	auto button = std::make_shared<Button> ();
	button->setLabel(label);
	button->setPosition(position);
	button->setSize(size);
	return button;
}

std::string Button::getLabel() const
{
	return label;
}

void Button::setLabel(const std::string &newLabel)
{
	label = newLabel;
}

bool Button::isButtonDown() const
{
	return isButtonDown_;
}

void Button::handleMouseButtonDown(MouseButtonEvent &event)
{
	Widget::handleMouseButtonDown(event);
	isButtonDown_ = true;
	captureMouse();
}

void Button::handleMouseButtonUp(MouseButtonEvent &event)
{
	Widget::handleMouseButtonUp(event);
	isButtonDown_ = false;
	releaseMouse(); 
}

void Button::drawContentOn(Canvas *canvas)
{
	if(isButtonDown())
	{
		canvas->setColor(glm::vec4(0.0,0.2,0.2, 1.0));
	}
	else
	{
		if(hasMouseOver())
			canvas->setColor(glm::vec4(0.0,0.2,0.8, 1.0));
		else
			canvas->setColor(glm::vec4(0.0,0.2,0.6, 1.0));
	}
	
	canvas->drawFillRoundedRectangle(getLocalRectangle(), 5);
	canvas->setColor(glm::vec4(0.0,0.8,0.8, 1.0));
	canvas->drawRoundedRectangle(getLocalRectangle(), 5);

    canvas->setColor(Colors::white());
    canvas->drawText(label, 12, glm::vec2(5, getHeight() - 5));
}

} // End of namespace GUI
} // End of namespace Loden
