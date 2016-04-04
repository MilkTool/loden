#ifndef LODEN_IMAGE_DRAWING_HPP
#define LODEN_IMAGE_DRAWING_HPP

#include "Loden/Image/ImageBuffer.hpp"
#include <string.h>

namespace Loden
{
namespace Image
{

template<typename DestPixelType>
void expandBitmap(int destX, int destY, ImageBuffer *dest, ImageBuffer *bitmap)
{
    auto black = DestPixelType::black();
    auto white = DestPixelType::white();

    auto destPitch = dest->getPitch();
    auto destRow = dest->get() + destX + destY*destPitch * sizeof(DestPixelType);

    auto srcRow = bitmap->get();
    auto copyHeight = bitmap->getHeight();
    auto copyPitch = bitmap->getPitch();
    auto copyWidth = bitmap->getWidth();
    for (int y = 0; y < copyHeight; ++y)
    {
        auto dst = reinterpret_cast<DestPixelType*> (destRow);
        int dstX = 0;
        for (int x = 0; x < copyPitch; ++x)
        {
            auto sample = srcRow[x];
            for (int i = 0; i < 8 && dstX < copyWidth; ++i, ++dstX)
            {
                if ((sample & (1 << (7 - i))) != 0)
                    dst[dstX] = white;
                else
                    dst[dstX] = black;
            }

        }

        srcRow += copyPitch;
        destRow += destPitch;
    }
}

template<typename PixelType>
void copyRectangle(int destX, int destY, ImageBuffer *dest, int sourceX, int sourceY, int width, int height, ImageBuffer *source)
{
    auto destPitch = dest->getPitch();
    auto destRow = dest->get() + destY*destPitch + destX * sizeof(PixelType);

    auto rowSize = width * sizeof(PixelType);
    auto sourcePitch = source->getPitch();
    auto sourceRow = source->get() + sourceY*sourcePitch + sourceX * sizeof(PixelType);

    for (int y = 0; y < height; ++y)
    {
        memcpy(destRow, sourceRow, rowSize);
        sourceRow += sourcePitch;
        destRow += destPitch;
    }
}

inline void clearImageBuffer(ImageBuffer *imageBuffer)
{
    memset(imageBuffer->get(), 0, imageBuffer->getSize());
}

} // End of namespace Image
} // End of namespace Loden

#endif //LODEN_IMAGE_DRAWING_HPP
