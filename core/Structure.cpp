#include "Loden/Structure.hpp"

namespace Loden
{
const StructureFieldTypeDescription StructureFieldTypeDescription::Descriptions[(int)StructureFieldType::Count]{
    {"float", AGPU_FLOAT, 1, 1, false, 4, 4},
    {"vec2", AGPU_FLOAT, 2, 1, false, 4, 8},
    {"vec3", AGPU_FLOAT, 3, 1, false, 4, 12},
    {"vec4", AGPU_FLOAT, 4, 1, false, 4, 16 },

    {"int", AGPU_INT, 1, 1, false, 4, 4},
    {"ivec2", AGPU_INT, 2, 1, false, 4, 8},
    {"ivec3", AGPU_INT, 3, 1, false, 4, 12},
    {"ivec4", AGPU_INT, 4, 1, false, 4, 16},

    {"uint", AGPU_UNSIGNED_INT, 1, 1, false, 4, 4},
    {"uivec2", AGPU_UNSIGNED_INT, 2, 1, false, 4, 8},
    {"uivec3", AGPU_UNSIGNED_INT, 3, 1, false, 4, 12},
    {"uivec4", AGPU_UNSIGNED_INT, 4, 1, false, 4, 16},

    {"short", AGPU_SHORT, 1, 1, false, 2, 2},
    {"svec2", AGPU_SHORT, 2, 1, false, 4, 4},
    {"svec4", AGPU_SHORT, 4, 1, false, 4, 8},

    {"ushort", AGPU_UNSIGNED_SHORT, 1, 1, false, 2, 2},
    {"usvec2", AGPU_UNSIGNED_SHORT, 2, 1, false, 4, 4},
    {"usvec4", AGPU_UNSIGNED_SHORT, 4, 1, false, 4, 8},

    {"nshort", AGPU_SHORT, 1, 1, true, 2, 2},
    {"nsvec2", AGPU_SHORT, 2, 1, true, 4, 4},
    {"nsvec4", AGPU_SHORT, 4, 1, true, 4, 8},

    {"nushort", AGPU_UNSIGNED_SHORT, 1, 1, true, 2, 2},
    {"nusvec2", AGPU_UNSIGNED_SHORT, 2, 1, true, 4, 4},
    {"nusvec4", AGPU_UNSIGNED_SHORT, 4, 1, true, 4, 8},

    {"byte", AGPU_BYTE, 1, 1, false, 1, 1},
    {"bvec2", AGPU_BYTE, 2, 1, false, 2, 2},
    {"bvec4", AGPU_BYTE, 4, 1, false, 4, 4},

    {"ubyte", AGPU_UNSIGNED_BYTE, 1, 1, false, 1, 1},
    {"ubvec2", AGPU_UNSIGNED_BYTE, 2, 1, false, 2, 2},
    {"ubvec4", AGPU_UNSIGNED_BYTE, 4, 1, false, 4, 4},

    {"nbyte", AGPU_BYTE, 1, 1, true, 1, 1},
    {"nbvec2", AGPU_BYTE, 2, 1, true, 2, 2},
    {"nbvec4", AGPU_BYTE, 4, 1, true, 4, 4},

    {"nubyte", AGPU_UNSIGNED_BYTE, 1, 1, true, 1, 1},
    {"nubvec2", AGPU_UNSIGNED_BYTE, 2, 1, true, 2, 2},
    {"nubvec4", AGPU_UNSIGNED_BYTE, 4, 1, true, 4, 4},
};

} // End of namespace Loden
