#ifndef LODEN_COLOR_HPP
#define LODEN_COLOR_HPP

#include "Loden/Common.hpp"
#include "Loden/Math.hpp"
#include <glm/vec4.hpp>
#include <math.h>

namespace Loden
{

namespace Colors
{

inline glm::vec4 red()
{
    return glm::vec4(1.0, 0.0, 0.0, 1.0);
}

inline glm::vec4 green()
{
    return glm::vec4(0.0, 1.0, 0.0, 1.0);
}

inline glm::vec4 blue()
{
    return glm::vec4(0.0, 0.0, 1.0, 1.0);
}

inline glm::vec4 yellow()
{
    return glm::vec4(1.0, 1.0, 0.0, 1.0);
}

inline glm::vec4 cyan()
{
    return glm::vec4(0.0, 1.0, 0.0, 1.0);
}

inline glm::vec4 magenta()
{
    return glm::vec4(1.0, 0.0, 1.0, 1.0);
}

inline glm::vec4 black()
{
    return glm::vec4(0.0, 0.0, 0.0, 1.0);
}

inline glm::vec4 white()
{
    return glm::vec4(1.0, 1.0, 1.0, 1.0);
}

inline glm::vec4 transparent()
{
    return glm::vec4(0.0, 0.0, 0.0, 0.0);
}
} // End of namespace Colors

inline float srgbToLrgb(float component)
{
    if (component <= 0.0031308f)
        return 12.92f*component;
    else
        return 1.055f*powf(component, 1.0f / 2.4f) - 0.055f;
}

inline float lrgbToSrgb(float component)
{
    if (component <= 0.04045f)
        return component / 12.92f;
    else
        return powf((component + 0.055f) / 1.055f, 2.4f);
}

inline glm::vec4 srgbToLrgb(const glm::vec4 &color)
{
    return glm::vec4(srgbToLrgb(color.r), srgbToLrgb(color.g), srgbToLrgb(color.b), color.a);
}

inline glm::vec4 lrgbToSrgb(const glm::vec4 &color)
{
    return glm::vec4(lrgbToSrgb(color.r), lrgbToSrgb(color.g), lrgbToSrgb(color.b), color.a);
}

inline bool isTransparentColor(const glm::vec4 &color)
{
    return !closeTo(color.a, 1.0);
}

inline bool isFullyTransparentColor(const glm::vec4 &color)
{
    return closeTo(color.a, 0.0);
}

} // End of namespace Loden

#endif //LODEN_COLOR_HPP