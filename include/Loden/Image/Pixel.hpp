#ifndef LODEN_IMAGE_PIXEL_HPP
#define LODEN_IMAGE_PIXEL_HPP

#include "Loden/Common.hpp"
#include "Loden/Image/PixelChannel.hpp"
#include <glm/vec4.hpp>
#include <stdint.h>

namespace Loden
{
namespace Image
{

template<typename CT>
struct PixelChannelCommon
{
    typedef CT ChannelType;
    typedef typename ColorTypeForEncodedChannelType<ChannelType>::type ColorType;
    typedef typename FloatTypeForEncodedChannelType<ChannelType>::type FloatType;

    static constexpr CT ChannelBlackValue = ChannelSpecialValues<ChannelType>::BlackValue;
    static constexpr CT ChannelWhiteValue = ChannelSpecialValues<ChannelType>::WhiteValue;
};

template<typename CT>
struct PixelRGBA : public PixelChannelCommon<CT>
{
    typedef PixelRGBA<CT> SelfType;

    PixelRGBA(ChannelType r=0, ChannelType g=0, ChannelType b=0, ChannelType a=0)
        : r(r), g(g), b(b), a(a) {}

    static SelfType transparent()
    {
        return SelfType(ChannelBlackValue, ChannelBlackValue, ChannelBlackValue, ChannelBlackValue);
    }

    static SelfType black()
    {
        return SelfType(ChannelBlackValue, ChannelBlackValue, ChannelBlackValue, ChannelWhiteValue);
    }

    static SelfType white()
    {
        return SelfType(ChannelWhiteValue, ChannelWhiteValue, ChannelWhiteValue, ChannelWhiteValue);
    }

    void setColor(const ColorType &color)
    {
        r = encodeNormalizedChannel<ChannelType> (color.r);
        g = encodeNormalizedChannel<ChannelType> (color.g);
        b = encodeNormalizedChannel<ChannelType> (color.b);
        a = encodeNormalizedChannel<ChannelType> (color.a);
    }

    ColorType asColor() const
    {
        return ColorType(
                decodeNormalizedChannel(r),
                decodeNormalizedChannel(g),
                decodeNormalizedChannel(b),
                decodeNormalizedChannel(a),
            );
    }

    void setVector(const ColorType &color)
    {
        setColor(color);
    }

    ColorType asVector() const
    {
        return asColor();
    }

    ChannelType r, g, b, a;
};

template<typename CT>
struct PixelR : public PixelChannelCommon<CT>
{
    typedef PixelR<CT> SelfType;

    PixelR(ChannelType r = 0)
        : r(r) {}

    static SelfType transparent()
    {
        return SelfType(ChannelBlackValue);
    }

    static SelfType black()
    {
        return SelfType(ChannelBlackValue);
    }

    static SelfType white()
    {
        return SelfType(ChannelWhiteValue);
    }

    void setColor(const ColorType &color)
    {
        r = encodeNormalizedChannel<ChannelType>(color.r);
    }

    ColorType asColor() const
    {
        return ColorType(decodeNormalizedChannel(r), 0, 0, 0);
    }

    void setVector(FloatType v)
    {
        r = encodeNormalizedChannel<ChannelType> (v);
    }

    FloatType asVector() const
    {
        return decodeNormalizedChannel(r);
    }

    ChannelType r;
};

template<typename CT>
struct PixelRG : public PixelChannelCommon<CT>
{
    typedef PixelRG<CT> SelfType;

    PixelRG(ChannelType r = 0, ChannelType g = 0)
        : r(r), g(g) {}

    static SelfType transparent()
    {
        return SelfType(ChannelBlackValue, ChannelBlackValue);
    }

    static SelfType black()
    {
        return SelfType(ChannelBlackValue, ChannelBlackValue);
    }

    static SelfType white()
    {
        return SelfType(ChannelWhiteValue, ChannelWhiteValue);
    }

    void setColor(const ColorType &color)
    {
        r = encodeNormalizedChannel<ChannelType>(color.r);
        g = encodeNormalizedChannel<ChannelType>(color.g);
    }

    ColorType asColor() const
    {
        return ColorType(
            decodeNormalizedChannel(r), 
            decodeNormalizedChannel(g),
            0,
            0);
    }

    ChannelType r, g;
};

typedef PixelRGBA<uint8_t> PixelRGBA8;
typedef PixelR<uint8_t> PixelR8;
typedef PixelRG<uint8_t> PixelRG8;

typedef PixelRGBA<int8_t> PixelRGBA8s;
typedef PixelR<int8_t> PixelR8s;
typedef PixelRG<int8_t> PixelRG8s;

typedef PixelRGBA<uint16_t> PixelRGBA16;
typedef PixelR<uint16_t> PixelR16;
typedef PixelRG<uint16_t> PixelRG16;

typedef PixelRGBA<int16_t> PixelRGBA16s;
typedef PixelR<int16_t> PixelR16s;
typedef PixelRG<int16_t> PixelRG16s;

typedef PixelRGBA<float> PixelRGBA32F;
typedef PixelR<float> PixelR32F;
typedef PixelRG<float> PixelRG32F;

typedef PixelRGBA<double> PixelRGBA64F;
typedef PixelR<double> PixelR64F;
typedef PixelRG<double> PixelRG64F;

} // End of namespace Image
} // End of namespace Loden

#endif //LODEN_IMAGE_PIXEL_HPP
