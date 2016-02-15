#ifndef LODEN_GUI_CANVAS_HPP_
#define LODEN_GUI_CANVAS_HPP_

#include "Loden/Common.hpp"
#include "Loden/Object.hpp"
#include "Loden/Rectangle.hpp"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <functional>

namespace Loden
{
namespace GUI
{
LODEN_DECLARE_CLASS(Font);
LODEN_DECLARE_CLASS(FontFace);

/**
 * Path fill rule.
 */
enum class PathFillRule
{
    EvenOdd = 0,
    NonZero,
    Convex,
};

/**
 * 2D Canvas rendering interface
 */
class LODEN_CORE_EXPORT Canvas: public ObjectInterfaceSubclass<Canvas, Object>
{
    LODEN_OBJECT_TYPE(Canvas)
public:
	virtual void setColor(const glm::vec4 &color) = 0;
	
	virtual void drawLine(const glm::vec2 &p1, const glm::vec2 &p2) = 0;
	virtual void drawTriangle(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3) = 0;
	virtual void drawRectangle(const Rectangle &rectangle) = 0;
    virtual void drawRoundedRectangle(const Rectangle &rectangle, float cornerRadius) = 0;

	virtual void drawFillTriangle(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3) = 0;
	virtual void drawFillRectangle(const Rectangle &rectangle) = 0;
    virtual void drawFillRoundedRectangle(const Rectangle &rectangle, float cornerRadius) = 0;

    // Text drawing
    virtual glm::vec2 drawText(const std::string &text, int pointSize, glm::vec2 position) = 0;
    virtual glm::vec2 drawTextUtf16(const std::wstring &text, int pointSize, glm::vec2 position) = 0;

    // Fill paths.
    virtual void beginFillPath(PathFillRule fillRule = PathFillRule::EvenOdd) = 0;
    virtual void closePath() = 0;
    virtual void moveTo(const glm::vec2 &point) = 0;
    virtual void lineTo(const glm::vec2 &point) = 0;
    virtual void quadTo(const glm::vec2 &control, const glm::vec2 &point) = 0;
    virtual void cubicTo(const glm::vec2 &control, const glm::vec2 &control2, const glm::vec2 &point) = 0;
    virtual void endFillPath() = 0;

    // Stroke paths
    virtual void beginStrokePath() = 0;
    virtual void endStrokePath() = 0;

    // Clip paths
    virtual void beginClipPath(PathFillRule fillRule = PathFillRule::EvenOdd) = 0;
    virtual void endClipPath() = 0;
    virtual void popClipPath() = 0;

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

    template<typename CP, typename FT>
    void withClipPath(const CP &clipPath, const FT &f)
    {
        beginClipPath();
        clipPath();
        endClipPath();
        f();
        popClipPath();
    }

    template<typename FT>
    void withClipRectangle(const Rectangle &rectangle, const FT &f)
    {
        withClipPath([&] {
            moveTo(rectangle.getBottomLeft());
            lineTo(rectangle.getBottomRight());
            lineTo(rectangle.getTopRight());
            lineTo(rectangle.getTopLeft());
            closePath();
        }, f);
    }
};

} // Loden
} // GUI

#endif //LODEN_GUI_CANVAS_HPP_
