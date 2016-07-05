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
    Float2,
    Float3,
    Float4,

    Int,
    Int2,
    Int3,
    Int4,

    UInt,
    UInt2,
    UInt3,
    UInt4,

    Short,
    Short2,
    Short4,

    UShort,
    UShort2,
    UShort4,

    Byte,
    Byte2,
    Byte4,

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
    agpu_texture_format format;

    static const StructureFieldTypeDescription Descriptions[(int)StructureFieldType::Count];
};

/**
 * Structure field
 */
class LODEN_CORE_EXPORT StructureField
{
public:
    StructureFieldType type;
    agpu_texture_format format;
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
    agpu_texture_format format;
    size_t size;
    size_t alignment;
    std::vector<StructureField> fields;
};

} // End of namespace Loden

#endif //LODEN_STRUCTURE_HPP_
