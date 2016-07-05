#include "Loden/TextureFormats.hpp"

namespace Loden
{

#define FormatDef(Name, hasColor, hasDepth, hasStencil, size, alignment) \
    {AGPU_TEXTURE_FORMAT_ ## Name, #Name, hasColor, hasDepth, hasStencil, size, alignment}

#define ColorFormatDef(Name, size, alignment) \
    FormatDef(Name, true, false, false, size, alignment)

#define DepthFormatDef(Name, size, alignment) \
    FormatDef(Name, false, true, false, size, alignment)

#define DepthStencilFormatDef(Name, size, alignment) \
    FormatDef(Name, false, true, true, size, alignment)

const TextureFormatDescription TextureFormatDescription::Descriptions[] = {
    FormatDef(UNKNOWN, false, false, false, 0, 1),
    ColorFormatDef(R32G32B32A32_TYPELESS, 16, 16),
    ColorFormatDef(R32G32B32A32_FLOAT, 16, 16),
    ColorFormatDef(R32G32B32A32_UINT, 16, 16),
    ColorFormatDef(R32G32B32A32_SINT, 16, 16),
    ColorFormatDef(R32G32B32_TYPELESS, 12, 16),
    ColorFormatDef(R32G32B32_FLOAT, 12, 16),
    ColorFormatDef(R32G32B32_UINT, 12, 16),
    ColorFormatDef(R32G32B32_SINT, 12, 16),
    ColorFormatDef(R16G16B16A16_TYPELESS, 8, 8),
    ColorFormatDef(R16G16B16A16_FLOAT, 8, 8),
    ColorFormatDef(R16G16B16A16_UNORM, 8, 8),
    ColorFormatDef(R16G16B16A16_UINT, 8, 8),
    ColorFormatDef(R16G16B16A16_SNORM, 8, 8),
    ColorFormatDef(R16G16B16A16_SINT, 8, 8),
    ColorFormatDef(R32G32_TYPELESS, 8, 8),
    ColorFormatDef(R32G32_FLOAT, 8, 8),
    ColorFormatDef(R32G32_UINT, 8, 8),
    ColorFormatDef(R32G32_SINT, 8, 8),
    ColorFormatDef(R32G8X24_TYPELESS, 8, 8),
    DepthStencilFormatDef(D32_FLOAT_S8X24_UINT, 8, 8),
    ColorFormatDef(R32_FLOAT_S8X24_TYPELESS, 8, 8),
    ColorFormatDef(X32_TYPELESS_G8X24_UINT, 8, 8),
    ColorFormatDef(R10G10B10A2_TYPELESS, 4, 4),
    ColorFormatDef(R10G10B10A2_UNORM, 4, 4),
    ColorFormatDef(R10G10B10A2_UINT, 4, 4),
    ColorFormatDef(R11G11B10_FLOAT, 4, 4),
    ColorFormatDef(R8G8B8A8_TYPELESS, 4, 4),
    ColorFormatDef(R8G8B8A8_UNORM, 4, 4),
    ColorFormatDef(R8G8B8A8_UNORM_SRGB, 4, 4),
    ColorFormatDef(R8G8B8A8_UINT, 4, 4),
    ColorFormatDef(R8G8B8A8_SNORM, 4, 4),
    ColorFormatDef(R8G8B8A8_SINT, 4, 4),
    ColorFormatDef(R16G16_TYPELESS, 4, 4),
    ColorFormatDef(R16G16_FLOAT, 4, 4),
    ColorFormatDef(R16G16_UNORM, 4, 4),
    ColorFormatDef(R16G16_UINT, 4, 4),
    ColorFormatDef(R16G16_SNORM, 4, 4),
    ColorFormatDef(R16G16_SINT, 4, 4),
    ColorFormatDef(R32_TYPELESS, 4, 4),
    DepthFormatDef(D32_FLOAT, 4, 4),
    ColorFormatDef(R32_FLOAT, 4, 4),
    ColorFormatDef(R32_UINT, 4, 4),
    ColorFormatDef(R32_SINT, 4, 4),
    ColorFormatDef(R24G8_TYPELESS, 4, 4),
    DepthStencilFormatDef(D24_UNORM_S8_UINT, 4, 4),
    ColorFormatDef(R24_UNORM_X8_TYPELESS, 4, 4),
    ColorFormatDef(X24TG8_UINT, 4, 4),
    ColorFormatDef(R8G8_TYPELESS, 2, 2),
    ColorFormatDef(R8G8_UNORM, 2, 2),
    ColorFormatDef(R8G8_UINT, 2, 2),
    ColorFormatDef(R8G8_SNORM, 2, 2),
    ColorFormatDef(R8G8_SINT, 2, 2),
    ColorFormatDef(R16_TYPELESS, 2, 2),
    ColorFormatDef(R16_FLOAT, 2, 2),
    DepthFormatDef(D16_UNORM, 2, 2),
    ColorFormatDef(R16_UNORM, 2, 2),
    ColorFormatDef(R16_UINT, 2, 2),
    ColorFormatDef(R16_SNORM, 2, 2),
    ColorFormatDef(R16_SINT, 2, 2),
    ColorFormatDef(R8_TYPELESS, 1, 1),
    ColorFormatDef(R8_UNORM, 1, 1),
    ColorFormatDef(R8_UINT, 1, 1),
    ColorFormatDef(R8_SNORM, 1, 1),
    ColorFormatDef(R8_SINT, 1, 1),
    ColorFormatDef(A8_UNORM, 1, 1),
    ColorFormatDef(R1_UNORM, 1, 1),
    ColorFormatDef(BC1_TYPELESS, 4, 4),
    ColorFormatDef(BC1_UNORM, 4, 4),
    ColorFormatDef(BC1_UNORM_SRGB, 4, 4),
    ColorFormatDef(BC2_TYPELESS, 4, 4),
    ColorFormatDef(BC2_UNORM, 4, 4),
    ColorFormatDef(BC2_UNORM_SRGB, 4, 4),
    ColorFormatDef(BC3_TYPELESS, 4, 4),
    ColorFormatDef(BC3_UNORM, 4, 4),
    ColorFormatDef(BC3_UNORM_SRGB, 4, 4),
    ColorFormatDef(BC4_TYPELESS, 4, 4),
    ColorFormatDef(BC4_UNORM, 4, 4),
    ColorFormatDef(BC4_SNORM, 4, 4),
    ColorFormatDef(BC5_TYPELESS, 4, 4),
    ColorFormatDef(BC5_UNORM, 4, 4),
    ColorFormatDef(BC5_SNORM, 4, 4),
    ColorFormatDef(B5G6R5_UNORM, 2, 2),
    ColorFormatDef(B5G5R5A1_UNORM, 2, 2),
    ColorFormatDef(B8G8R8A8_UNORM, 4, 4),
    ColorFormatDef(B8G8R8X8_UNORM, 4, 4),
    ColorFormatDef(B8G8R8A8_TYPELESS, 4, 4),
    ColorFormatDef(B8G8R8A8_UNORM_SRGB, 4, 4),
    ColorFormatDef(B8G8R8X8_TYPELESS, 4, 4),
    ColorFormatDef(B8G8R8X8_UNORM_SRGB, 4, 4),

    {AGPU_TEXTURE_FORMAT_UNKNOWN, nullptr, false, false, false, 0, 1},
};

} // End of namespace Loden
