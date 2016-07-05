#ifndef LODEN_MATRICES_HPP
#define LODEN_MATRICES_HPP

#include "Loden/Common.hpp"
#include <glm/mat4x4.hpp>

namespace Loden
{

/**
 * Constructs an orthographic matrix
 */
LODEN_CORE_EXPORT glm::mat4 orthographicMatrix(float left, float right, float bottom, float top, float near, float far, bool invertedY);

/**
 * Constructs an orthographic matrix with reverse depth mapping.
 */
LODEN_CORE_EXPORT glm::mat4 reverseOrthographicMatrix(float left, float right, float bottom, float top, float near, float far, bool invertedY);

/**
 * Constructs a perspective frustum matrix.
 */
LODEN_CORE_EXPORT glm::mat4 frustumMatrix(float left, float right, float bottom, float top, float near, float far, bool invertedY);

/**
* Constructs a perspective frustum matrix with reverse depth mapping.
*/
LODEN_CORE_EXPORT glm::mat4 reverseFrustumMatrix(float left, float right, float bottom, float top, float near, float far, bool invertedY);

} // End of namespace Loden

#endif //LODEN_MATRICES_HPP
