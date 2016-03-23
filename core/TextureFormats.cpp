#include "Loden/TextureFormats.hpp"

namespace Loden
{

#define FormatDef(Name, hasColor, hasDepth, hasStencil) \
    {AGPU_TEXTURE_FORMAT_ ## Name, #Name, hasColor, hasDepth, hasStencil}

#define ColorFormatDef(Name) \
    FormatDef(Name, true, false, false)

#define DepthFormatDef(Name) \
    FormatDef(Name, false, true, false)

#define DepthStencilFormatDef(Name) \
    FormatDef(Name, false, true, true)

const TextureFormatDescription TextureFormatDescription::Descriptions[] = {
    FormatDef(UNKNOWN, false, false, false),
    ColorFormatDef(R32G32B32A32_TYPELESS),
    ColorFormatDef(R32G32B32A32_FLOAT),
    ColorFormatDef(R32G32B32A32_UINT),
    ColorFormatDef(R32G32B32A32_SINT),
    ColorFormatDef(R32G32B32_TYPELESS),
    ColorFormatDef(R32G32B32_FLOAT),
    ColorFormatDef(R32G32B32_UINT),
    ColorFormatDef(R32G32B32_SINT),
    ColorFormatDef(R16G16B16A16_TYPELESS),
    ColorFormatDef(R16G16B16A16_FLOAT),
    ColorFormatDef(R16G16B16A16_UNORM),
    ColorFormatDef(R16G16B16A16_UINT),
    ColorFormatDef(R16G16B16A16_SNORM),
    ColorFormatDef(R16G16B16A16_SINT),
    ColorFormatDef(R32G32_TYPELESS),
    ColorFormatDef(R32G32_FLOAT),
    ColorFormatDef(R32G32_UINT),
    ColorFormatDef(R32G32_SINT),
    ColorFormatDef(R32G8X24_TYPELESS),
    DepthStencilFormatDef(D32_FLOAT_S8X24_UINT),
    ColorFormatDef(R32_FLOAT_S8X24_TYPELESS),
    ColorFormatDef(X32_TYPELESS_G8X24_UINT),
    ColorFormatDef(R10G10B10A2_TYPELESS),
    ColorFormatDef(R10G10B10A2_UNORM),
    ColorFormatDef(R10G10B10A2_UINT),
    ColorFormatDef(R11G11B10A2_FLOAT),
    ColorFormatDef(R8G8B8A8_TYPELESS),
    ColorFormatDef(R8G8B8A8_UNORM),
    ColorFormatDef(R8G8B8A8_UNORM_SRGB),
    ColorFormatDef(R8G8B8A8_UINT),
    ColorFormatDef(R8G8B8A8_SNORM),
    ColorFormatDef(R8G8B8A8_SINT),
    ColorFormatDef(R16G16_TYPELESS),
    ColorFormatDef(R16G16_FLOAT),
    ColorFormatDef(R16G16_UNORM),
    ColorFormatDef(R16G16_UINT),
    ColorFormatDef(R16G16_SNORM),
    ColorFormatDef(R16G16_SINT),
    ColorFormatDef(R32_TYPELESS),
    DepthFormatDef(D32_FLOAT),
    ColorFormatDef(R32_FLOAT),
    ColorFormatDef(R32_UINT),
    ColorFormatDef(R32_SINT),
    ColorFormatDef(R24G8_TYPELESS),
    DepthStencilFormatDef(D24_UNORM_S8_UINT),
    ColorFormatDef(R24_UNORM_X8_TYPELESS),
    ColorFormatDef(X24TG8_UINT),
    ColorFormatDef(R8G8_TYPELESS),
    ColorFormatDef(R8G8_UNORM),
    ColorFormatDef(R8G8_UINT),
    ColorFormatDef(R8G8_SNORM),
    ColorFormatDef(R8G8_SINT),
    ColorFormatDef(R16_TYPELESS),
    ColorFormatDef(R16_FLOAT),
    DepthFormatDef(D16_UNORM),
    ColorFormatDef(R16_UNORM),
    ColorFormatDef(R16_UINT),
    ColorFormatDef(R16_SNORM),
    ColorFormatDef(R16_SINT),
    ColorFormatDef(R8_TYPELESS),
    ColorFormatDef(R8_UNORM),
    ColorFormatDef(R8_UINT),
    ColorFormatDef(R8_SNORM),
    ColorFormatDef(R8_SINT),
    ColorFormatDef(A8_UNORM),
    ColorFormatDef(R1_UNORM),
    ColorFormatDef(BC1_TYPELESS),
    ColorFormatDef(BC1_UNORM),
    ColorFormatDef(BC1_UNORM_SRGB),
    ColorFormatDef(BC2_TYPELESS),
    ColorFormatDef(BC2_UNORM),
    ColorFormatDef(BC2_UNORM_SRGB),
    ColorFormatDef(BC3_TYPELESS),
    ColorFormatDef(BC3_UNORM),
    ColorFormatDef(BC3_UNORM_SRGB),
    ColorFormatDef(BC4_TYPELESS),
    ColorFormatDef(BC4_UNORM),
    ColorFormatDef(BC4_SNORM),
    ColorFormatDef(BC5_TYPELESS),
    ColorFormatDef(BC5_UNORM),
    ColorFormatDef(BC5_SNORM),
    ColorFormatDef(B5G6R5_UNORM),
    ColorFormatDef(B5G5R5A1_UNORM),
    ColorFormatDef(B8G8R8A8_UNORM),
    ColorFormatDef(B8G8R8X8_UNORM),
    ColorFormatDef(B8G8R8A8_TYPELESS),
    ColorFormatDef(B8G8R8A8_UNORM_SRGB),
    ColorFormatDef(B8G8R8X8_TYPELESS),
    ColorFormatDef(B8G8R8X8_UNORM_SRGB),

    {AGPU_TEXTURE_FORMAT_UNKNOWN, nullptr, false, false, false},
};

} // End of namespace Loden
