#include "ir/common.h"
#include "ir/instruction.h"
#include "utils/macros.h"

namespace compiler::ir {

std::ostream &operator<<(std::ostream &os, const Opcode &op)
{
    switch (op) {
        case Opcode::PARAMETER:
            os << "Parameter";
            break;
        case Opcode::CONSTANT:
            os << "Constant";
            break;
        case Opcode::ADD:
            os << "Add";
            break;
        case Opcode::MUL:
            os << "Mul";
            break;
        case Opcode::SHL:
            os << "Shl";
            break;
        case Opcode::XOR:
            os << "Xor";
            break;
        case Opcode::COMPARE:
            os << "Compare";
            break;
        case Opcode::BRANCH:
            os << "Br";
            break;
        case Opcode::COND_BRANCH:
            os << "If";
            break;
        case Opcode::RETURN:
            os << "Return";
            break;
        case Opcode::PHI:
            os << "Phi";
            break;
        case Opcode::MEM:
            os << "Mem";
            break;
        case Opcode::LOAD:
            os << "Load";
            break;
        case Opcode::STORE:
            os << "Store";
            break;
        case Opcode::CHECK:
            os << "Check";
            break;
        case Opcode::CALL_STATIC:
            os << "CallSt";
            break;
        default:
            UNREACHABLE();
            break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const ResultType &resType)
{
    switch (resType) {
        case ResultType::VOID:
            break;
        case ResultType::BOOL:
            os << "b";
            break;
        case ResultType::S8:
            os << "s8";
            break;
        case ResultType::U8:
            os << "u8";
            break;
        case ResultType::S16:
            os << "s16";
            break;
        case ResultType::U16:
            os << "u16";
            break;
        case ResultType::S32:
            os << "s32";
            break;
        case ResultType::U32:
            os << "u32";
            break;
        case ResultType::S64:
            os << "s64";
            break;
        case ResultType::U64:
            os << "u64";
            break;
        default:
            UNREACHABLE();
            break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const CmpFlags &cmpFlags)
{
    switch (cmpFlags) {
        case CmpFlags::LE:
            os << "LE";
            break;
        case CmpFlags::LT:
            os << "LT";
            break;
        default:
            UNREACHABLE();
            break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const CheckType &checkType)
{
    switch (checkType) {
        case CheckType::NIL:
            os << "Nil";
            break;
        case CheckType::BOUND:
            os << "Bound";
            break;
        default:
            UNREACHABLE();
            break;
    }
    return os;
}

ResultType CombineResultType(const Instruction *op1, const Instruction *op2)
{
    auto resType1 = static_cast<int>(op1->GetResultType());
    auto resType2 = static_cast<int>(op2->GetResultType());
    return static_cast<ResultType>(std::max(resType1, resType2));
}

bool operator<=(ResultType resType1, ResultType resType2)
{
    auto t1 = static_cast<int>(resType1);
    auto t2 = static_cast<int>(resType2);
    return t1 <= t2;
}

}  // namespace compiler::ir
