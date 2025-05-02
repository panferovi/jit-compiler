#ifndef IR_COMMON_H
#define IR_COMMON_H

#include <cstdint>
#include <ostream>

namespace compiler::ir {

class Instruction;

enum class Opcode : uint32_t {
    PARAMETER,
    CONSTANT,
    ADD,
    MUL,
    SHL,
    XOR,
    COMPARE,
    BRANCH,
    COND_BRANCH,
    RETURN,
    PHI,
    COUNT,
    INVALID
};

// Order has meaning (in increase)
enum class ResultType { VOID, BOOL, S8, U8, S16, U16, S32, U32, S64, U64, INVALID };

enum class CmpFlags { LE, LT, INVALID };

std::ostream &operator<<(std::ostream &os, const Opcode &op);
std::ostream &operator<<(std::ostream &os, const ResultType &resType);
std::ostream &operator<<(std::ostream &os, const CmpFlags &cmpFlags);

template <Opcode op>
static constexpr uint32_t OpcodeToIndex()
{
    return static_cast<uint32_t>(op);
}

ResultType CombineResultType(const Instruction *op1, const Instruction *op2);

}  // namespace compiler::ir

#endif
