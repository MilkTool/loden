#include "Loden/Structure.hpp"

namespace Loden
{
const StructureFieldTypeDescription StructureFieldTypeDescription::Descriptions[(int)StructureFieldType::Count]{
    {"float", AGPU_FLOAT, 1, 1, false, 4, 4, AGPU_TEXTURE_FORMAT_R32_FLOAT},
    {"float2", AGPU_FLOAT, 2, 1, false, 4, 8, AGPU_TEXTURE_FORMAT_R32G32_FLOAT},
    {"float3", AGPU_FLOAT, 3, 1, false, 4, 16, AGPU_TEXTURE_FORMAT_R32G32B32_FLOAT},
    {"float4", AGPU_FLOAT, 4, 1, false, 4, 16, AGPU_TEXTURE_FORMAT_R32G32B32A32_FLOAT},

    {"int", AGPU_INT, 1, 1, false, 4, 4, AGPU_TEXTURE_FORMAT_R32_SINT},
    {"int2", AGPU_INT, 2, 1, false, 4, 8, AGPU_TEXTURE_FORMAT_R32G32_SINT},
    {"int3", AGPU_INT, 3, 1, false, 4, 16, AGPU_TEXTURE_FORMAT_R32G32B32_SINT},
    {"int4", AGPU_INT, 4, 1, false, 4, 16, AGPU_TEXTURE_FORMAT_R32G32B32A32_SINT},

    {"uint", AGPU_UNSIGNED_INT, 1, 1, false, 4, 4, AGPU_TEXTURE_FORMAT_R32_UINT},
    {"uint2", AGPU_UNSIGNED_INT, 2, 1, false, 4, 8, AGPU_TEXTURE_FORMAT_R32G32_UINT},
    {"uint3", AGPU_UNSIGNED_INT, 3, 1, false, 4, 16, AGPU_TEXTURE_FORMAT_R32G32B32_UINT},
    {"uint4", AGPU_UNSIGNED_INT, 4, 1, false, 4, 16, AGPU_TEXTURE_FORMAT_R32G32B32A32_UINT},

    {"short", AGPU_SHORT, 1, 1, false, 2, 2, AGPU_TEXTURE_FORMAT_R16_SINT},
    {"short2", AGPU_SHORT, 2, 1, false, 4, 4, AGPU_TEXTURE_FORMAT_R16G16_SINT},
    {"short4", AGPU_SHORT, 4, 1, false, 4, 8, AGPU_TEXTURE_FORMAT_R16G16B16A16_SINT},

    {"ushort", AGPU_UNSIGNED_SHORT, 1, 1, false, 2, 2, AGPU_TEXTURE_FORMAT_R16_UINT},
    {"ushort2", AGPU_UNSIGNED_SHORT, 2, 1, false, 4, 4, AGPU_TEXTURE_FORMAT_R16G16_UINT},
    {"ushort4", AGPU_UNSIGNED_SHORT, 4, 1, false, 4, 8, AGPU_TEXTURE_FORMAT_R16G16B16A16_UINT},

    {"byte", AGPU_BYTE, 1, 1, false, 1, 1, AGPU_TEXTURE_FORMAT_R8_UINT},
    {"byte2", AGPU_BYTE, 2, 1, false, 2, 2, AGPU_TEXTURE_FORMAT_R8G8_UINT},
    {"byte4", AGPU_BYTE, 4, 1, false, 4, 4, AGPU_TEXTURE_FORMAT_R8G8B8A8_UINT},
};

} // End of namespace Loden
