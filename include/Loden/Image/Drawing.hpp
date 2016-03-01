#ifndef LODEN_IMAGE_DRAWING_HPP
#define LODEN_IMAGE_DRAWING_HPP

#include "Loden/Image/ImageBuffer.hpp"

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
    auto destRow = dest->get() + destX + destY*destPitch;

    auto srcRow = bitmap->get();
    auto copyHeight = bitmap->getHeight();
    auto copyPitch = bitmap->getPitch();
    for (int y = 0; y < copyHeight; ++y)
    {
        auto dst = reinterpret_cast<DestPixelType*> (destRow);
        for (int x = 0; x < copyPitch; ++x)
        {
            auto sample = srcRow[x];
            for (int i = 0; i < 8; ++i, ++dst)
            {
                if ((sample & (1 << (7 - i))) != 0)
                    *dst = white;
                else
                    *dst = black;
            }
                
        }

        srcRow += copyPitch;
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
