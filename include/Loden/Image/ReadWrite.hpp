#ifndef LODEN_IMAGE_READ_WRITE_HPP
#define LODEN_IMAGE_READ_WRITE_HPP

#include "Loden/Image/ImageBuffer.hpp"
#include <string>

namespace Loden
{
namespace Image
{

LODEN_CORE_EXPORT ImageBufferPtr loadImageFromPng(const std::string &fileName);

LODEN_CORE_EXPORT bool saveImageAsPng(const std::string &fileName, ImageBuffer *imageBuffer);

} // End of namespace Image
} // End of namespace Loden

#endif //LODEN_IMAGE_READ_WRITE_HPP
