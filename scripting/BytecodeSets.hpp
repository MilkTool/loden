#ifndef LODTALK_BYTECODE_SETS_HPP
#define LODTALK_BYTECODE_SETS_HPP

namespace Lodtalk
{

/**
 * SistaV1 bytecode set.
 */
namespace SistaV1BytecodeSet
{
#define SISTAV1_INSTRUCTION_RANGE(name, range_first, range_end) \
	constexpr int name##First = range_first; \
	constexpr int name##Last = range_end; \
	constexpr int name##RangeSize = range_end - range_first + 1;
	
#define SISTAV1_INSTRUCTION(name, opcode) \
	constexpr int name = opcode; 

#include "SistaV1BytecodeSet.inc"

#undef SISTAV1_INSTRUCTION_RANGE
#undef SISTAV1_INSTRUCTION
};

namespace BytecodeSet = SistaV1BytecodeSet;

} // End of namespace Lodtalk

#endif //LODTALK_BYTECODE_SETS_HPP
