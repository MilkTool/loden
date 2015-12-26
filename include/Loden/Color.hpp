#ifndef LODEN_COLOR_HPP
#define LODEN_COLOR_HPP

#include "Loden/Common.hpp"
#include <glm/vec4.hpp>

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

} // End of namespace Colors


} // End of namespace Loden

#endif //LODEN_COLOR_HPP