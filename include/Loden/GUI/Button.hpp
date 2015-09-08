#ifndef LODEN_GUI_BUTTON_HPP
#define LODEN_GUI_BUTTON_HPP

#include "Loden/GUI/Widget.hpp"

namespace Loden
{
namespace GUI
{

LODEN_DECLARE_CLASS(Button);

/**
 * Button widget
 */
class LODEN_CORE_EXPORT Button: public Widget
{
public:
	Button();
	~Button();
	
	static ButtonPtr create(const std::string &label, const glm::vec2 &size, const glm::vec2 &position = glm::vec2());
	
	std::string getLabel() const;
	void setLabel(const std::string &newLabel);
	
	bool isButtonDown() const;
	
	virtual void drawContentOn(Canvas *canvas);

	virtual void handleMouseButtonDown(MouseButtonEvent &event);
	virtual void handleMouseButtonUp(MouseButtonEvent &event);

private:
	std::string label;
	bool isButtonDown_;
	
};
} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_BUTTON_HPP
