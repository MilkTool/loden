#ifndef LODEN_IMAGE_PIXEL_CHANNEL_HPP
#define LODEN_IMAGE_PIXEL_CHANNEL_HPP

#include "Loden/Common.hpp"
#include "Loden/Math.hpp"
#include <limits>
#include <glm/vec4.hpp>

namespace Loden
{
namespace Image
{

template<typename T>
struct FloatTypeForEncodedChannelType
{
    typedef float type;
};

template<>
struct FloatTypeForEncodedChannelType<double>
{
    typedef double type;
};

template<typename T>
struct ColorTypeForEncodedChannelType
{
    typedef glm::vec4 type;
};

template<>
struct ColorTypeForEncodedChannelType<double>
{
    typedef glm::dvec4 type;
};

template<typename T>
struct NormalizedChannelDecoder;

template<>
struct NormalizedChannelDecoder<uint8_t>
{
    static constexpr float apply(uint8_t v)
    {
        return float(v) / float(UINT8_MAX);
    }
};

template<>
struct NormalizedChannelDecoder<uint16_t>
{
    static constexpr float apply(uint16_t v)
    {
        return float(v) / float(UINT16_MAX);
    }
};

template<>
struct NormalizedChannelDecoder<uint32_t>
{
    static constexpr float apply(uint32_t v)
    {
        return float(v) / float(UINT32_MAX);
    }
};

template<>
struct NormalizedChannelDecoder<int8_t>
{
    static constexpr float apply(int8_t v)
    {
        return float(v) / float(INT8_MAX);
    }
};

template<>
struct NormalizedChannelDecoder<int16_t>
{
    static constexpr float apply(int16_t v)
    {
        return float(v) / float(INT16_MAX);
    }
};

template<>
struct NormalizedChannelDecoder<int32_t>
{
    static constexpr double apply(int32_t v)
    {
        return double(v) / double(INT32_MAX);
    }
};

template<>
struct NormalizedChannelDecoder<float>
{
    static constexpr float apply(float v)
    {
        return v;
    }
};

template<>
struct NormalizedChannelDecoder<double>
{
    static constexpr double apply(double v)
    {
        return v;
    }
};

template<typename T>
struct NormalizedChannelEncoder;

template<>
struct NormalizedChannelEncoder<uint8_t>
{
    static uint8_t apply(float v)
    {
        return (uint8_t)clamp<uint32_t> (0, UINT8_MAX, uint32_t(UINT8_MAX*v));
    }
};

template<>
struct NormalizedChannelEncoder<uint16_t>
{
    static uint16_t apply(float v)
    {
        return (uint16_t)clamp<uint32_t> (0, UINT16_MAX, uint32_t(UINT16_MAX*v));
    }
};

template<>
struct NormalizedChannelEncoder<uint32_t>
{
    static uint32_t apply(double v)
    {
        return clamp<uint32_t> (0, UINT32_MAX, uint32_t(UINT32_MAX*v));
    }
};

template<>
struct NormalizedChannelEncoder<int8_t>
{
    static int8_t apply(float v)
    {
        return (int8_t)clamp<int32_t> (-INT8_MAX, INT8_MAX, int32_t(INT8_MAX*v));
    }
};

template<>
struct NormalizedChannelEncoder<int16_t>
{
    static int16_t apply(float v)
    {
        return (int16_t)clamp<int32_t> (-INT16_MAX, INT16_MAX, int32_t(INT16_MAX*v));
    }
};

template<>
struct NormalizedChannelEncoder<int32_t>
{
    static int32_t apply(double v)
    {
        return clamp<int32_t> (-INT32_MAX, INT32_MAX, int32_t(INT32_MAX*v));
    }
};

template<>
struct NormalizedChannelEncoder<float>
{
    static constexpr float apply(float v)
    {
        return v;
    }
};

template<>
struct NormalizedChannelEncoder<double>
{
    static constexpr double apply(double v)
    {
        return v;
    }
};

template<typename Result, typename V>
Result encodeNormalizedChannel(V v)
{
    return NormalizedChannelEncoder<Result>::apply(v);
}

template<typename V>
auto decodeNormalizedChannel(V v) -> decltype(NormalizedChannelDecoder<V>::apply(v))
{
    return NormalizedChannelDecoder<V>::apply(v);
}

template<typename T>
struct ChannelSpecialValues
{
    static constexpr T BlackValue = 0;
    static constexpr T WhiteValue = std::numeric_limits<T>::max();
};

template<>
struct ChannelSpecialValues<float>
{
    static constexpr float BlackValue = 0.0f;
    static constexpr float WhiteValue = 1.0f;
};

template<>
struct ChannelSpecialValues<double>
{
    static constexpr double BlackValue = 0.0;
    static constexpr double WhiteValue = 1.0;
};

template<typename T>
struct SaturateChannel;

template<>
struct SaturateChannel<uint8_t>
{
    static uint8_t apply(float v)
    {
        return (uint8_t)clamp<float>(0, UINT8_MAX, v);
    }
};

template<>
struct SaturateChannel<uint16_t>
{
    static uint16_t apply(float v)
    {
        return (uint16_t)clamp<float>(0, UINT16_MAX, v);
    }
};

template<>
struct SaturateChannel<uint32_t>
{
    static uint32_t apply(double v)
    {
        return (uint32_t)clamp<double>(0, UINT32_MAX, v);
    }
};

template<>
struct SaturateChannel<int8_t>
{
    static int8_t apply(float v)
    {
        return (int8_t) clamp<float>(-INT8_MAX, INT8_MAX, v);
    }
};

template<>
struct SaturateChannel<int16_t>
{
    static int16_t apply(float v)
    {
        return (int16_t)clamp<float>(-INT16_MAX, INT16_MAX, v);
    }
};

template<>
struct SaturateChannel<int32_t>
{
    static int32_t apply(double v)
    {
        return (int32_t)clamp<double>(-INT32_MAX, INT32_MAX, v);
    }
};

} // End of namespace Image
} // End of namespace Loden

#endif //LODEN_IMAGE_PIXEL_CHANNEL_HPP
