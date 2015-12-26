#ifndef LODEN_MATH_HPP
#define LODEN_MATH_HPP

#include "Loden/Common.hpp"

namespace Loden
{

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

} // End of namesapce Loden

#endif //LODEN_MATH_HPP
