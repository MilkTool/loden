#ifndef LODEN_SIGNED_DISTANCE_FIELD_TRANSFORM_HPP
#define LODEN_SIGNED_DISTANCE_FIELD_TRANSFORM_HPP

#include "Loden/Image/ImageBuffer.hpp"
#include "Loden/Image/Drawing.hpp"
#include <assert.h>

namespace Loden
{
namespace Image
{

template<typename PixelType>
void computeSignedDistanceField(ImageBuffer *dest, ImageBuffer *source, float distanceScaleFactor)
{
    assert(dest->getWidth() == source->getWidth());
    assert(dest->getHeight() == source->getHeight());
    assert(dest->getPitch() == source->getPitch());

    int height = (int)source->getHeight();
    int width = (int)source->getWidth();
    auto pitch = source->getPitch();
    auto sourceData = source->get();

    auto destData = dest->get();

    // Special handling for the all zero.
    bool isAllZero = true;
    for (size_t i = 0; i < source->getSize(); ++i)
    {
        if (sourceData[i] != 0)
        {
            isAllZero = false;
            break;
        }
    }

    if (isAllZero)
    {
        clearImageBuffer(dest);
        return;
    }

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            auto sample = reinterpret_cast<PixelType*> (sourceData + y*pitch + x)->r;
            float bestDistance = INFINITY;
            float sign = (sample == 0) ? -1.0f : 1.0f;

            for (int sy = 0; sy < height; ++sy)
            {
                if (y == sy)
                    continue;

                float dy = float(sy - y);
                if (dy > bestDistance || -dy > bestDistance)
                    continue;

                auto startX = floor(std::max(0.0f, x - bestDistance));

                for (int sx = int(startX); sx < width; ++sx)
                {
                    if (x == sx)
                        continue;

                    float dx = float(sx - x);
                    if (dx > bestDistance)
                        break;

                    auto testSample = reinterpret_cast<PixelType*> (sourceData + sy*pitch + sx)->r;
                    if (testSample == sample)
                        continue;

                    float newDistance = sqrt(dx*dx + dy*dy);
                    if (newDistance < bestDistance)
                        bestDistance =  newDistance;

                }
            }

            reinterpret_cast<PixelType*> (destData + y*pitch + x)->r = PixelType::saturateChannel(sign*bestDistance*distanceScaleFactor);
        }
    }
}

template<typename PixelType>
void computeSmallerDistanceField(ImageBuffer *dest, ImageBuffer *source, float distanceScaleFactor)
{
    assert(dest->getWidth() <= source->getWidth());
    assert(dest->getHeight() <= source->getHeight());
    assert(dest->getPitch() <= source->getPitch());

    int destHeight = (int)dest->getHeight();
    int destWidth = (int)dest->getWidth();
    auto destPitch = dest->getPitch();
    auto destData = dest->get();

    int height = (int)source->getHeight();
    int width = (int)source->getWidth();
    auto pitch = source->getPitch();
    auto sourceData = source->get();

    auto xScaleFactor = float(width-1) / std::max(1.0f, float(destWidth-1));
    auto yScaleFactor = float(height-1) / std::max(1.0f, float(destHeight-1));

    // Special handling for the all zero.
    bool isAllZero = true;
    for (size_t i = 0; i < source->getSize(); ++i)
    {
        if (sourceData[i] != 0)
        {
            isAllZero = false;
            break;
        }
    }

    if (isAllZero)
    {
        clearImageBuffer(dest);
        return;
    }

    for (int dy = 0; dy < destHeight; ++dy)
    {
        for (int dx = 0; dx < destWidth; ++dx)
        {
            auto x = int(dx*xScaleFactor);
            auto y = int(dy*yScaleFactor);

            auto sample = reinterpret_cast<PixelType*> (sourceData + y*pitch + x)->r;
            float bestDistance = INFINITY;
            float sign = (sample == 0) ? -1.0f : 1.0f;

            for (int sy = 0; sy < height; ++sy)
            {
                if (y == sy)
                    continue;

                float dy = float(sy - y);
                if (dy > bestDistance || -dy > bestDistance)
                    continue;

                auto startX = floor(std::max(0.0f, x - bestDistance));

                for (int sx = int(startX); sx < width; ++sx)
                {
                    if (x == sx)
                        continue;

                    float dx = float(sx - x);
                    if (dx > bestDistance)
                        break;

                    auto testSample = reinterpret_cast<PixelType*> (sourceData + sy*pitch + sx)->r;
                    if (testSample == sample)
                        continue;

                    float newDistance = sqrt(dx*dx + dy*dy);
                    if (newDistance < bestDistance)
                        bestDistance =  newDistance;

                }
            }

            reinterpret_cast<PixelType*> (destData + dy*destPitch + dx)->r = PixelType::saturateChannel(sign*bestDistance*distanceScaleFactor);
        }
    }
}

} // End of namespace Image
} // End of namespace Loden

#endif //LODEN_SIGNED_DISTANCE_FIELD_TRANSFORM_HPP
