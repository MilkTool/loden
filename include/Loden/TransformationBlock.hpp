#ifndef LODEN_TRANSFORMATION_BLOCK_HPP
#define LODEN_TRANSFORMATION_BLOCK_HPP

#include <glm/mat4x4.hpp>

/**
 * The transformation block layout
 */
struct TransformationBlock
{
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 modelMatrix;
};

#endif // LODEN_TRANSFORMATION_BLOCK_HPP
