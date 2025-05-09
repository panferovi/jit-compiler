#include "ir/instruction.h"
#include "ir/common.h"
#include "ir/basic_block.h"

namespace compiler::ir {

namespace {

void DumpInputs(std::stringstream &ss, const Instruction::Inputs &inputs)
{
    for (auto inputIt = inputs.cbegin(), inputEnd = inputs.cend(); inputIt != inputEnd; ++inputIt) {
        ss << 'v' << (*inputIt)->GetInstId().GetId();
        if (std::next(inputIt) != inputEnd) {
            ss << ", ";
        }
    }
}

}  // namespace

void Instruction::Dump(std::stringstream &ss) const
{
    ss << instId_ << '.' << resType_ << ' ' << op_ << ' ';
}

/* static */
void Instruction::UpdateUsersAndEliminate(Instruction *inst, Instruction *newInst)
{
    ASSERT(inst != newInst);
    ASSERT(newInst != nullptr);

    auto &users = inst->users_;
    newInst->AddUsers(users);
    for (auto *user : users) {
        if (user->GetOpcode() == Opcode::PHI) {
            user->As<PhiInst>()->UpdateDependencies(inst, newInst);
        } else {
            user->UpdateInputs(inst, newInst);
        }
    }
    users.clear();
    Instruction::Eliminate(inst);
}

/* static */
void Instruction::Eliminate(Instruction *inst)
{
    ASSERT(inst->GetUsers().empty());
    if (inst->GetOpcode() == Opcode::PHI) {
        for (auto &[input, _] : inst->As<PhiInst>()->GetValueDependencies()) {
            input->users_.erase(inst);
        }
    } else {
        for (auto *input : inst->GetInputs()) {
            input->users_.erase(inst);
        }
    }
    inst->Unlink();
    delete inst;
}

void Instruction::UpdateInputs(Instruction *oldInput, Instruction *newInput)
{
    for (auto &input : inputs_) {
        if (input == oldInput) {
            input = newInput;
            break;
        }
    }
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

std::pair<bool, bool> ArithmInst::CheckInputsAreConst()
{
    auto *op1 = GetInputs().front();
    auto *op2 = GetInputs().back();
    return {op1->GetOpcode() == ir::Opcode::CONSTANT, op2->GetOpcode() == ir::Opcode::CONSTANT};
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
    if (GetResultType() == ResultType::VOID) {
        ss << "void";
    } else {
        ss << 'v' << GetFirstOp()->GetInstId().GetId();
    }
}

void PhiInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    for (auto valueDepIt = valueDeps_.cbegin(); valueDepIt != valueDeps_.cend(); ++valueDepIt) {
        auto &[inst, bbs] = *valueDepIt;
        for (auto bbIt = bbs.cbegin(); bbIt != bbs.cend(); ++bbIt) {
            ss << 'v' << inst->GetInstId().GetId() << ":BB." << (*bbIt)->GetId();
            if (std::next(bbIt) != bbs.cend()) {
                ss << ", ";
            }
        }
        if (std::next(valueDepIt) != valueDeps_.cend()) {
            ss << ", ";
        }
    }
}

void PhiInst::ResolveDependency(Instruction *value, BasicBlock *bb)
{
    ASSERT(value->GetResultType() == GetResultType());
    valueDeps_[value].push_back(bb);
    value->AddUsers(this);
}

void PhiInst::UpdateDependencies(Instruction *oldValue, Instruction *newValue)
{
    auto node = valueDeps_.extract(oldValue);
    ASSERT(!node.empty());
    node.key() = newValue;
    auto insertRes = valueDeps_.insert(std::move(node));
    if (!insertRes.inserted) {
        valueDeps_[newValue].merge(std::move(insertRes.node.mapped()));
    }
}

bool PhiInst::HasOnlyOneDependency()
{
    return valueDeps_.size() == 1;
}

void MemoryInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    ss << 'v' << GetFirstOp()->GetInstId().GetId();
}

void LoadInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    ss << 'v' << GetFirstOp()->GetInstId().GetId() << ", v" << GetLastOp()->GetInstId().GetId();
}

void StoreInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    DumpInputs(ss, GetInputs());
}

void CheckInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    ss << GetType() << " ";
    DumpInputs(ss, GetInputs());
}

}  // namespace compiler::ir
