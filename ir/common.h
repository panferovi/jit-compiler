#ifndef IR_COMMON_H
#define IR_COMMON_H

#include <ostream>

namespace compiler::ir {

enum class Opcode { PARAMETER, CONSTANT, ADD, MUL, COMPARE, BRANCH, COND_BRANCH, RETURN, PHI, INVALID };

// Order has meaning (in increase)
enum class ResultType { VOID, BOOL, S8, U8, S16, U16, S32, U32, S64, U64, INVALID };

enum class CmpFlags { LE, LT, INVALID };

std::ostream &operator<<(std::ostream &os, const Opcode &op);
std::ostream &operator<<(std::ostream &os, const ResultType &resType);
std::ostream &operator<<(std::ostream &os, const CmpFlags &cmpFlags);

}  // namespace compiler::ir

#endif
