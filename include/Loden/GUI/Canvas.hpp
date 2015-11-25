#ifndef LODEN_GUI_CANVAS_HPP_
#define LODEN_GUI_CANVAS_HPP_

#include "Loden/Common.hpp"
#include "Loden/Rectangle.hpp"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <functional>

namespace Loden
{
namespace GUI
{

/**
 * 2D Canvas rendering interface
 */
class Canvas
{
public:
	virtual void setColor(const glm::vec4 &color) = 0;
	
	virtual void drawLine(const glm::vec2 &p1, const glm::vec2 &p2) = 0;
	virtual void drawTriangle(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3) = 0;
	virtual void drawRectangle(const Rectangle &rectangle) = 0;

	virtual void drawFillTriangle(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3) = 0;
	virtual void drawFillRectangle(const Rectangle &rectangle) = 0;
	
	virtual const glm::mat3 &getTransform() const = 0;
	virtual void setTransform(const glm::mat3 &transform) = 0;

	template<typename FT>	
	void withTranslation(const glm::vec2 &translation, const FT &f)
	{
		auto oldTransform = getTransform();
		auto newTransform = oldTransform * glm::mat3(glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(translation.x, translation.y, 1.0));
		setTransform(newTransform);
		f();
		setTransform(oldTransform);
	}
};

} // Loden
} // GUI

#endif //LODEN_GUI_CANVAS_HPP_
