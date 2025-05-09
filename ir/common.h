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
    MEM,
    LOAD,
    STORE,
    CHECK,
    COUNT,
    INVALID
};

// Order has meaning (in increase)
enum class ResultType { VOID, BOOL, S8, U8, S16, U16, S32, U32, S64, U64, INVALID };

enum class CmpFlags { LE, LT, INVALID };

enum class CheckType : uint32_t { NIL, BOUND, COUNT };

std::ostream &operator<<(std::ostream &os, const Opcode &op);
std::ostream &operator<<(std::ostream &os, const ResultType &resType);
std::ostream &operator<<(std::ostream &os, const CmpFlags &cmpFlags);
std::ostream &operator<<(std::ostream &os, const CheckType &checkType);

template <Opcode op>
static constexpr uint32_t OpcodeToIndex()
{
    return static_cast<uint32_t>(op);
}

static constexpr uint32_t OpcodeToIndex(Opcode op)
{
    return static_cast<uint32_t>(op);
}

template <CheckType type>
static constexpr uint32_t CheckTypeToIndex()
{
    return static_cast<uint32_t>(type);
}

static constexpr uint32_t CheckTypeToIndex(CheckType type)
{
    return static_cast<uint32_t>(type);
}

ResultType CombineResultType(const Instruction *op1, const Instruction *op2);

bool operator<=(ResultType resType1, ResultType resType2);

}  // namespace compiler::ir

#endif
