#ifndef LODEN_RECTANGLE_HPP
#define LODEN_RECTANGLE_HPP

#include "Loden/Common.hpp"
#include <glm/vec2.hpp>

namespace Loden
{

/**
 * Rectangle class
 */
class Rectangle
{
public:
    Rectangle() {};
    Rectangle(const glm::vec2 &min, const glm::vec2 &max)
        : min(min), max(max) {}
    ~Rectangle() {}

    bool containsPoint(const glm::vec2 &point) const
    {
        return min.x <= point.x && point.x <= max.x &&
            min.y <= point.y && point.y <= max.y;
    }

    bool containsRectangle(const Rectangle &other) const
    {
        return min.x <= other.min.x && other.max.x <= max.x &&
            min.y <= other.min.y && other.max.y <= max.y;
    }

    bool isOutside(const Rectangle &other) const
    {
        return other.max.x < min.x || other.min.x > max.x ||
            other.max.y < min.y || other.min.y > max.y;
    }

    bool intersectsOrContains(const Rectangle &other) const
    {
        return !isOutside(other);
    }

    bool intersects(const Rectangle &other) const
    {
        return !isOutside(other) && !containsRectangle(other) && other.containsRectangle(*this);
    }

    glm::vec2 getBottomLeft() const
    {
        return min;
    }

    glm::vec2 getBottomRight() const
    {
        return glm::vec2(max.x, min.y);
    }

    glm::vec2 getTopLeft() const
    {
        return glm::vec2(min.x, max.y);
    }

    glm::vec2 getTopRight() const
    {
        return max;
    }

    glm::vec2 min, max;
};

} // End of namespace Loden

#endif //LODEN_RECTANGLE_HPP