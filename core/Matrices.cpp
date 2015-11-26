#include "Loden/Matrices.hpp"

namespace Loden
{

LODEN_CORE_EXPORT glm::mat4 orthographicMatrix(float left, float right, float bottom, float top, float near, float far)
{
    glm::mat4 mat;
    mat[0][0] = 2.0f / (right - left); mat[1][0] = 0.0f; mat[2][0] = 0.0f;  mat[3][0] = -(right + left) / (right - left);
    mat[0][1] = 0.0f; mat[1][1] = 2.0f / (top - bottom); mat[2][1] = 0.0f;  mat[3][1] = -(top + bottom) / (top - bottom);
    mat[0][2] = 0.0f; mat[1][2] = 0.0f; mat[2][2] = -1.0f / (far - near);   mat[3][2] = -near / (far - near);
    mat[0][3] = 0.0f; mat[1][3] = 0.0f; mat[2][3] = 0.0f;                   mat[3][3] = 1.0f;
    return mat;
}

LODEN_CORE_EXPORT glm::mat4 reverseOrthographicMatrix(float left, float right, float bottom, float top, float near, float far)
{
    glm::mat4 mat;
    mat[0][0] = 2.0f / (right - left); mat[1][0] = 0.0f; mat[2][0] = 0.0f; mat[3][0] = -(right + left) / (right - left);
    mat[0][1] = 0.0f; mat[1][1] = 2.0f / (top - bottom); mat[2][1] = 0.0f; mat[3][1] = -(top + bottom) / (top - bottom);
    mat[0][2] = 0.0f; mat[1][2] = 0.0f; mat[2][2] = 1.0f / (far - near);   mat[3][2] = far / (far - near);
    mat[0][3] = 0.0f; mat[1][3] = 0.0f; mat[2][3] = 0.0f;                  mat[3][3] = 1.0f;
    return mat;
}

LODEN_CORE_EXPORT glm::mat4 frustumMatrix(float left, float right, float bottom, float top, float near, float far)
{
    glm::mat4 mat;
    mat[0][0] = 2.0f*near / (right - left); mat[1][0] = 0.0f; mat[2][0] = (left + right) / (right - left); mat[3][0] = 0.0f;
    mat[0][1] = 0.0f; mat[1][1] = 2.0f*near / (top - bottom); mat[2][1] = (top + bottom) / (top - bottom); mat[3][1] = 0.0f;
    mat[0][2] = 0.0f; mat[1][2] = 0.0f; mat[2][2] = -far / (far - near); mat[3][2] = -near*far/(far - near);
    mat[0][3] = 0.0f; mat[1][3] = 0.0f; mat[2][3] = -1.0f; mat[3][3] = 0.0f;
    return mat;
}

LODEN_CORE_EXPORT glm::mat4 reverseFrustumMatrix(float left, float right, float bottom, float top, float near, float far)
{
    glm::mat4 mat;
    mat[0][0] = 2.0f*near / (right - left); mat[1][0] = 0.0f; mat[2][0] = (left + right) / (right - left); mat[3][0] = 0.0f;
    mat[0][1] = 0.0f; mat[1][1] = 2.0f*near / (top - bottom); mat[2][1] = (top + bottom) / (top - bottom); mat[3][1] = 0.0f;
    mat[0][2] = 0.0f; mat[1][2] = 0.0f; mat[2][2] = near / (far - near); mat[3][2] = near*far / (far - near);
    mat[0][3] = 0.0f; mat[1][3] = 0.0f; mat[2][3] = -1.0f; mat[3][3] = 0.0f;
    return mat;

}

} // End of namespace Loden
