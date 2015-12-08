#ifndef LODEN_STRUCTURE_HPP_
#define LODEN_STRUCTURE_HPP_

#include "Loden/Common.hpp"
#include "AGPU/agpu.hpp"
#include <string>
#include <vector>

namespace Loden
{

LODEN_DECLARE_CLASS(Structure);

/**
 * Structure field attribute type.
 */
enum class StructureFieldType
{
    Float = 0,
    Vec2,
    Vec3,
    Vec4,

    Int,
    IVec2,
    IVec3,
    IVec4,

    UInt,
    UIVec2,
    UIVec3,
    UIVec4,

    Short,
    SVec2,
    SVec4,

    UShort,
    USVec2,
    USVec4,

    NShort,
    NSVec2,
    NSVec4,

    NUShort,
    NUSVec2,
    NUSVec4,

    Byte,
    BVec2,
    BVec4,

    UByte,
    UBVec2,
    UBVec4,

    NByte,
    NBVec2,
    NBVec4,

    NUByte,
    NUBVec2,
    NUBVec4,

    Count
};

/**
 * Structure type
 */
enum class StructureType
{
    Generic = 0,
    Vertex,
    UniformState,
};

/**
 * Structure attribute type description
 */
class LODEN_CORE_EXPORT StructureFieldTypeDescription
{
public:
    const char *name;
    agpu_field_type type;
    agpu_uint components;
    agpu_uint rows;
    agpu_bool normalized;
    agpu_uint size;
    agpu_uint alignment;

    static const StructureFieldTypeDescription Descriptions[(int)StructureFieldType::Count];
};

/**
 * Structure field
 */
class LODEN_CORE_EXPORT StructureField
{
public:
    StructureFieldType type;
    std::string name;
    int binding;
    size_t offset;
};

/**
 * Structure
 */
class LODEN_CORE_EXPORT Structure
{
public:
    StructureType type;
    size_t size;
    size_t alignment;
    std::vector<StructureField> fields;
};

} // End of namespace Loden

#endif //LODEN_STRUCTURE_HPP_
