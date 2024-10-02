#include "ir/instruction.h"
#include "ir/common.h"
#include "ir/basic_block.h"

namespace compiler::ir {

void Instruction::Dump(std::stringstream &ss) const
{
    ss << instId_ << '.' << resType_ << ' ' << op_ << ' ';
}

void AssignInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    ss << value_;
}

void ArithmInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    auto &inputs = GetInputs();
    ss << 'v' << inputs.front()->GetInstId().GetId() << ", v" << inputs.back()->GetInstId().GetId();
}

void LogicInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    ss << flags_ << ' ';
    auto &inputs = GetInputs();
    ss << 'v' << inputs.front()->GetInstId().GetId() << ", v" << inputs.back()->GetInstId().GetId();
}

void BranchInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    auto *bb = GetBasicBlock();
    if (GetOpcode() == Opcode::BRANCH) {
        ss << "BB." << bb->GetTrueSuccessor()->GetId();
    } else {
        ASSERT(GetOpcode() == Opcode::COND_BRANCH);
        ss << 'v' << GetInputs().front()->GetInstId().GetId() << ", BB." << bb->GetTrueSuccessor()->GetId() << ", BB."
           << bb->GetFalseSuccessor()->GetId();
    }
}

void ReturnInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    auto &inputs = GetInputs();
    ss << 'v' << inputs.front()->GetInstId().GetId();
}

void PhiInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    for (auto valueDepIt = valueDeps_.cbegin(); valueDepIt != valueDeps_.cend(); ++valueDepIt) {
        ss << 'v' << valueDepIt->first->GetInstId().GetId() << ":BB." << valueDepIt->second->GetId();
        if (std::next(valueDepIt) != valueDeps_.cend()) {
            ss << ", ";
        }
    }
}

}  // namespace compiler::ir
