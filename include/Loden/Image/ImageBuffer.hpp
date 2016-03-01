#ifndef LODEN_IMAGE_IMAGE_BUFFER_HPP
#define LODEN_IMAGE_IMAGE_BUFFER_HPP

#include "Loden/Object.hpp"
#include "Loden/Image/Pixel.hpp"
#include <algorithm>

namespace Loden
{
namespace Image
{
/**
 * Image buffer
 */
class ImageBuffer
{
public:
    ImageBuffer(size_t width, size_t height, size_t pitch)
        : width(width), height(height), pitch(pitch)
    {
        size = pitch*height;
    }
    virtual ~ImageBuffer() {}

    size_t getSize() const
    {
        return size;
    }

    size_t getWidth() const
    {
        return width;
    }

    size_t getHeight() const
    {
        return height;
    }

    size_t getPitch() const
    {
        return pitch;
    }

    virtual uint8_t *get() = 0;

private:
    size_t size;
    size_t width;
    size_t height;
    size_t pitch;
};

/**
 * External image
 */
class ExternalImageBuffer : public ImageBuffer
{
public:
    ExternalImageBuffer(size_t width, size_t height, size_t pitch, uint8_t *buffer)
        : ImageBuffer(width, height, pitch), buffer(buffer)
    {
    }

    ~ExternalImageBuffer()
    {
    }
    
    virtual uint8_t *get() override
    {
        return buffer;
    }

private:

    uint8_t *buffer;

};

/**
 * Local image
 */
class LocalImageBuffer : public ImageBuffer
{
public:
    LocalImageBuffer(size_t width, size_t height, size_t pitch)
        : ImageBuffer(width, height, pitch)
    {
        buffer.reset(new uint8_t[getSize()]);
    }

    virtual uint8_t *get() override
    {
        return buffer.get();
    }

private:

    std::unique_ptr<uint8_t[]> buffer;
};


/**
 * Double image buffer
 */
class DoubleImageBuffer : public ImageBuffer
{
public:
    DoubleImageBuffer(size_t width, size_t height, size_t pitch)
        : ImageBuffer(width, height, pitch),
        buffer1(width, height, pitch),
        buffer2(width, height, pitch)
    {
        frontBuffer = &buffer1;
        backBuffer = &buffer2;
    }

    virtual uint8_t *get() override
    {
        return frontBuffer->get();
    }

    LocalImageBuffer *getFrontBuffer()
    {
        return frontBuffer;
    }

    LocalImageBuffer *getBackBuffer()
    {
        return frontBuffer;
    }

    void swap()
    {
        std::swap(frontBuffer, backBuffer);
    }

private:
    LocalImageBuffer buffer1, buffer2;
    LocalImageBuffer *frontBuffer;
    LocalImageBuffer *backBuffer;
};

} // End of namespace Image
} // End of namespace Loden

#endif //LODEN_IMAGE_IMAGE_BUFFER_HPP