#ifndef LODEN_TEXTURE_FORMATS_HPP
#define LODEN_TEXTURE_FORMATS_HPP

#include "Loden/Common.hpp"
#include "AGPU/agpu.hpp"

namespace Loden
{

/**
 * Texture format description;
 */
struct LODEN_CORE_EXPORT TextureFormatDescription
{
    agpu_texture_format format;
    const char *name;
    bool hasColor;
    bool hasDepth;
    bool hasStencil;
    agpu_size size;
    agpu_size alignment;
    static const TextureFormatDescription Descriptions[];
};

} // End of namespace Loden

#endif //LODEN_TEXTURE_FORMATS_HPP
