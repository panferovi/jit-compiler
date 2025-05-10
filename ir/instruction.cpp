#include "ir/instruction.h"
#include "ir/common.h"
#include "ir/basic_block.h"

#include <sstream>
#include <utility>

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

template <typename InstType, typename... InstArgs>
Instruction *CreateInstruction(InstArgs... args)
{
    auto *inst = new InstType {std::forward<InstArgs>(args)...};
    if (inst->GetOpcode() != Opcode::PHI) {
        inst->GetBasicBlock()->InsertInstBack(inst);
    } else {
        inst->GetBasicBlock()->InsertPhiInst(inst);
    }
    return inst;
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

void Instruction::UpdateBasicBlock(BasicBlock *newBB)
{
    ASSERT(ownBB_ != nullptr);
    ASSERT(newBB != nullptr);
    auto *oldBB = std::exchange(ownBB_, newBB);
    for (auto *user : GetUsers()) {
        if (user->GetOpcode() == ir::Opcode::PHI) {
            user->As<ir::PhiInst>()->UpdateValueBasicBlock(this, oldBB, newBB);
        }
    }
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

Instruction *Instruction::GetInput(size_t idx) const
{
    ASSERT(idx < inputs_.size());
    return *std::next(inputs_.begin(), idx);
}

void AssignInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    ss << value_;
}

Instruction *AssignInst::ShallowCopy(BasicBlock *newBB, InstId id) const
{
    return CreateInstruction<AssignInst>(newBB, id, GetOpcode(), GetResultType(), GetValue());
}

void ArithmInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    auto &inputs = GetInputs();
    ss << 'v' << inputs.front()->GetInstId().GetId() << ", v" << inputs.back()->GetInstId().GetId();
}

Instruction *ArithmInst::ShallowCopy(BasicBlock *newBB, InstId id) const
{
    return CreateInstruction<ArithmInst>(newBB, id, GetOpcode(), GetResultType(), InstProxyList {});
}

std::pair<bool, bool> ArithmInst::CheckInputsAreConst()
{
    auto *op1 = GetInputs().front();
    auto *op2 = GetInputs().back();
    return {op1->GetOpcode() == Opcode::CONSTANT, op2->GetOpcode() == Opcode::CONSTANT};
}

void LogicInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    ss << flags_ << ' ';
    auto &inputs = GetInputs();
    ss << 'v' << inputs.front()->GetInstId().GetId() << ", v" << inputs.back()->GetInstId().GetId();
}

Instruction *LogicInst::ShallowCopy(BasicBlock *newBB, InstId id) const
{
    return CreateInstruction<LogicInst>(newBB, id, GetOpcode(), InstProxyList {}, GetCmpFlags());
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

Instruction *BranchInst::ShallowCopy(BasicBlock *newBB, InstId id) const
{
    return CreateInstruction<BranchInst>(newBB, id, GetOpcode(), InstProxyList {});
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

Instruction *ReturnInst::ShallowCopy(BasicBlock *newBB, InstId id) const
{
    return CreateInstruction<ReturnInst>(newBB, id, GetResultType(), InstProxyList {});
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

Instruction *PhiInst::ShallowCopy(BasicBlock *newBB, InstId id) const
{
    return CreateInstruction<PhiInst>(newBB, id, GetResultType());
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

void PhiInst::UpdateValueBasicBlock(Instruction *value, BasicBlock *oldBB, BasicBlock *newBB)
{
    auto valueDepIt = valueDeps_.find(value);
    ASSERT(valueDepIt != valueDeps_.end());
    for (auto &bb : valueDepIt->second) {
        if (bb == oldBB) {
            bb = newBB;
        }
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

Instruction *MemoryInst::ShallowCopy(BasicBlock *newBB, InstId id) const
{
    return CreateInstruction<MemoryInst>(newBB, id, GetResultType(), InstProxyList {});
}

void LoadInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    ss << 'v' << GetFirstOp()->GetInstId().GetId() << ", v" << GetLastOp()->GetInstId().GetId();
}

Instruction *LoadInst::ShallowCopy(BasicBlock *newBB, InstId id) const
{
    return CreateInstruction<LoadInst>(newBB, id, GetResultType(), InstProxyList {});
}

void StoreInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    DumpInputs(ss, GetInputs());
}

Instruction *StoreInst::ShallowCopy(BasicBlock *newBB, InstId id) const
{
    return CreateInstruction<StoreInst>(newBB, id, InstProxyList {});
}

void CheckInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    ss << GetCheckType() << " ";
    DumpInputs(ss, GetInputs());
}

Instruction *CheckInst::ShallowCopy(BasicBlock *newBB, InstId id) const
{
    return CreateInstruction<CheckInst>(newBB, id, InstProxyList {}, GetCheckType());
}

void CallStaticInst::Dump(std::stringstream &ss) const
{
    Instruction::Dump(ss);
    ss << "id: " << GetCalleeId() << " Ret: " << GetResultType() << ' ';
    DumpInputs(ss, GetInputs());
}

Instruction *CallStaticInst::ShallowCopy(BasicBlock *newBB, InstId id) const
{
    return CreateInstruction<CallStaticInst>(newBB, id, GetResultType(), InstProxyList {}, GetCalleeId());
}

}  // namespace compiler::ir
