#ifndef LODEN_IMAGE_IMAGE_BUFFER_HPP
#define LODEN_IMAGE_IMAGE_BUFFER_HPP

#include "Loden/Object.hpp"
#include "Loden/Image/Pixel.hpp"
#include <algorithm>
#include <glm/glm.hpp>

namespace Loden
{
namespace Image
{
LODEN_DECLARE_CLASS(ImageBuffer);

/**
 * Image buffer
 */
class ImageBuffer
{
public:
    ImageBuffer(size_t width, size_t height, uint32_t bpp, ptrdiff_t pitch)
        : width(width), height(height), bpp(bpp), pitch(pitch)
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

    ptrdiff_t getPitch() const
    {
        return pitch;
    }

    ptrdiff_t getSlicePitch() const
    {
        return pitch*height;
    }

    uint32_t getBitsPerPixel() const
    {
        return bpp;
    }

    virtual uint8_t *get() = 0;

private:
    size_t size;
    size_t width;
    size_t height;
    uint32_t bpp;
    ptrdiff_t pitch;
};

/**
 * External image
 */
class ExternalImageBuffer : public ImageBuffer
{
public:
    ExternalImageBuffer(size_t width, size_t height, uint32_t bpp, ptrdiff_t pitch, uint8_t *buffer)
        : ImageBuffer(width, height, bpp, pitch), buffer(buffer)
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
    LocalImageBuffer(size_t width, size_t height, uint32_t bpp, ptrdiff_t pitch)
        : ImageBuffer(width, height, bpp, pitch)
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
    DoubleImageBuffer(size_t width, size_t height, uint32_t bpp, ptrdiff_t pitch)
        : ImageBuffer(width, height, bpp, pitch),
        buffer1(width, height, bpp, pitch),
        buffer2(width, height, bpp, pitch)
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
        return backBuffer;
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

class ImageSampler
{
public:
    ImageSampler(ImageBuffer *buffer)
        : data(buffer->get()), pitch(buffer->getPitch()), width(buffer->getWidth() - 1), height(buffer->getHeight() - 1)
    {
    }

    ImageSampler(ImageBuffer *buffer, int width, int height)
        : data(buffer->get()), pitch(buffer->getPitch()), width(width - 1), height(height - 1)
    {
    }

    template<typename PixelType>
    PixelType &at(int x, int y)
    {
        return *reinterpret_cast<PixelType*> (data + pitch*y + x);
    }

    template<typename PixelType>
    PixelType &at(const glm::ivec2 &coord)
    {
        return at<PixelType> (clampX(coord.x), clampY(coord.y));
    }

    template<typename PixelType>
    PixelType &at(const glm::vec2 &coord)
    {
        return at<PixelType> (glm::ivec2(noormalizedToPixels(coord)));
    }

    template<typename PixelType>
    PixelType &atPixel(const glm::vec2 &coord)
    {
        return at<PixelType> (glm::ivec2(coord));
    }

    template<typename PixelType>
    auto bilinearAt(const glm::vec2 &coord) -> decltype(PixelType().asVector())
    {
        auto centerCoord = noormalizedToPixels(coord);
        auto bottomLeftCoord = glm::floor(centerCoord);
        auto topRightCoord = glm::ceil(centerCoord);
        auto bottomLeft = atPixel<PixelType> (bottomLeftCoord).asVector();
        auto bottomRight = atPixel<PixelType> (glm::vec2(topRightCoord.x, bottomLeftCoord.y)).asVector();
        auto topRight = atPixel<PixelType> (topRightCoord).asVector();
        auto topLeft = atPixel<PixelType> (glm::vec2(bottomLeftCoord.x, topRightCoord.y)).asVector();
        auto fractCoord = centerCoord - bottomLeftCoord;
        auto top = glm::mix(topLeft, topRight, fractCoord.x);
        auto bottom = glm::mix(bottomLeft, bottomRight, fractCoord.x);
        return glm::mix(bottom, top, fractCoord.y);
    }

    glm::vec2 noormalizedToPixels(const glm::vec2 &coord)
    {
        return coord * glm::vec2(width, height);
    }

    int clampX(int coord) const
    {
        return std::max(0, std::min(coord, width));
    }

    int clampY(int coord) const
    {
        return std::max(0, std::min(coord, height));
    }
private:
    uint8_t *data;
    ptrdiff_t pitch;
    int width;
    int height;
};

} // End of namespace Image
} // End of namespace Loden

#endif //LODEN_IMAGE_IMAGE_BUFFER_HPP
