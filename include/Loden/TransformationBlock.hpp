#ifndef LODEN_TRANSFORMATION_BLOCK_HPP
#define LODEN_TRANSFORMATION_BLOCK_HPP

#include "Loden/Common.hpp"
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
static constexpr size_t TransformationBlock_AlignedSize = (sizeof(TransformationBlock) + 0xFF) & (~0xFF);

#endif // LODEN_TRANSFORMATION_BLOCK_HPP
