#ifndef LODEN_MATH_HPP
#define LODEN_MATH_HPP

#include "Loden/Common.hpp"
#include <algorithm>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Loden
{
static constexpr float FloatEpsilon = 0.00001f;

template<typename T, typename S>
T quadraticBezier(const T &P1, const T &P2, const T &P3, const S alpha)
{
    S ialpha = S(1) - alpha;
    S c1 = ialpha*ialpha;
    S c2 = 2*alpha*ialpha;
    S c3 = alpha*alpha;

    return c1*P1 + c2*P2 + c3*P3;
}

template<typename T, typename S>
T cubicBezier(const T &P1, const T &P2, const T &P3, const T &P4, const S alpha)
{
    S ialpha = S(1) - alpha;
    S c1 = ialpha*ialpha*ialpha;
    S c2 = 3*alpha*ialpha*ialpha;
    S c3 = 3*alpha*alpha*ialpha;
    S c4 = alpha*alpha*alpha;

    return c1*P1 + c2*P2 + c3*P3 + c4*P4;
}

template<typename T, typename S>
T quadraticRationalBezier(const T &P1, const S w1, const T &P2, const S w2, const T &P3, const S w3, const S alpha)
{
    S ialpha = S(1) - alpha;
    S c1 = ialpha*ialpha*w1;
    S c2 = 2 * alpha*ialpha*w2;
    S c3 = alpha*alpha*w3;
    S den = 1.0 / (c1 + c2 + c3);
    c1 *= den; c2 *= den; c3 *= den;

    return c1*P1 + c2*P2 + c3*P3;
}

template<typename T, typename S>
T cubicRationalBezier(const T &P1, const S w1, const T &P2, const S w2, const T &P3, const S w3, const T &P4, const S w4, const S alpha)
{
    S ialpha = S(1) - alpha;
    S c1 = ialpha*ialpha*ialpha*w1;
    S c2 = 3 * alpha*ialpha*ialpha*w2;
    S c3 = 3 * alpha*alpha*ialpha*w3;
    S c4 = alpha*alpha*alpha*w4;
    S den = 1.0 / (c1 + c2 + c3 + c4);
    c1 *= den; c2 *= den; c3 *= den; c4 *= den;

    return c1*P1 + c2*P2 + c3*P3 + c4*P4;
}

template<typename T>
T midpoint(const T &a, const T &b)
{
    return (a + b) * 0.5f;
}

inline bool closeTo(float a, float b)
{
    auto delta = a - b;
    return -FloatEpsilon < delta && delta < FloatEpsilon;
}

inline bool closeTo(const glm::vec2 &a, const glm::vec2 &b)
{
    return closeTo(a.x, b.x) && closeTo(a.y, b.y);
}

inline bool closeTo(const glm::vec3 &a, const glm::vec3 &b)
{
    return closeTo(a.x, b.x) && closeTo(a.y, b.y) && closeTo(a.z, b.z);
}

inline bool closeTo(const glm::vec4 &a, const glm::vec4 &b)
{
    return closeTo(a.x, b.x) && closeTo(a.y, b.y) && closeTo(a.z, b.z) && closeTo(a.w, b.w);
}

template<typename T>
inline const T clamp(T min, T max, T value)
{
    return std::min(max, std::max(min, value));
}

inline int integerLog2(int v)
{
    int result = 0;
    do
    {
        ++result;
        v >>= 1;
    } while (v != 0);

    return result-1;
}

inline int sameOrNextPowerOfTwo(int v)
{
    auto log2 = integerLog2(v);
    if (v <= (1 << log2))
        return (1 << log2);
    return 1 << (log2 + 1);
}

} // End of namesapce Loden

#endif //LODEN_MATH_HPP
