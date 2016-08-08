#ifndef LODEN_IMAGE_DOWNSAMPLE_HPP
#define LODEN_IMAGE_DOWNSAMPLE_HPP

#include "Loden/Image/ImageBuffer.hpp"
#include <glm/glm.hpp>

namespace Loden
{
namespace Image
{

template<typename PixelType>
void downsampleHalf(ImageBuffer *dest, ImageBuffer *source, size_t sourceWidth, size_t sourceHeight)
{
    auto destPitch = dest->getPitch();
    auto destRow = dest->get();

    auto sourceRow = source->get();
    auto sourcePitch = source->getPitch();

    auto sourceTop = sourceRow;
    auto sourceBottom = sourceRow + sourcePitch;
    auto sourcePitch2 = sourcePitch * 2;

    auto width = sourceWidth / 2;
    auto height = sourceHeight / 2;

    for (auto y = 0; y < height; ++y)
    {
        auto dst = reinterpret_cast<PixelType*> (destRow);
        auto top = reinterpret_cast<PixelType*> (sourceTop);
        auto bottom = reinterpret_cast<PixelType*> (sourceBottom);
        for (auto x = 0; x < width; ++x)
        {
            auto topLeft = top[x * 2].asVector();
            auto topRight = top[x * 2 + 1].asVector();
            auto bottomLeft = bottom[x * 2].asVector();
            auto bottomRight = bottom[x * 2 + 1].asVector();
            dst[x].setVector((topLeft + topRight + bottomLeft + bottomRight) / 4);
        }

        destRow += destPitch;
        sourceTop += sourcePitch2;
        sourceBottom += sourcePitch2;
    }
}

template<typename PixelType>
void downsample(DoubleImageBuffer *dest, ImageBuffer *source, int factor)
{
    if (factor <= 1)
        return;

    auto width = source->getWidth();
    auto height = source->getHeight();
    downsampleHalf<PixelType>(dest->getBackBuffer(), source, width, height);
    dest->swap();
    factor /= 2;
    width /= 2;
    height /= 2;

    while (factor > 1)
    {
        downsampleHalf<PixelType>(dest->getBackBuffer(), dest->getFrontBuffer(), width, height);
        dest->swap();
        factor /= 2;
        width /= 2;
        height /= 2;
    }
}

template<typename PixelType>
void linearScale(ImageBuffer *dest, int destWidth, int destHeight, ImageBuffer *source, int sourceWidth, int sourceHeight)
{
    auto destPitch = dest->getPitch();
    auto destRow = dest->get();

    glm::vec2 destFactor = glm::vec2(1.0 / (destWidth - 1), 1.0 / (destHeight - 1));

    ImageSampler sourceSampler(source, sourceWidth, sourceHeight);

    for(int dy = 0; dy < destHeight; ++dy, destRow += destPitch)
    {
        auto dst = reinterpret_cast<PixelType*> (destRow);

        for(int dx = 0; dx < destWidth; ++dx)
        {
            auto coord = destFactor * glm::vec2(dx, dy);
            auto value = sourceSampler.bilinearAt<PixelType> (coord);
            dst[dx].setVector(value);
        }
    }
}

} // End of namespace Image
} // End of namespace Loden

#endif //LODEN_IMAGE_DOWNSAMPLE_HPP
