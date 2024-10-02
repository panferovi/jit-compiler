#include "ir/basic_block.h"
#include "ir/instruction.h"

namespace compiler::ir {

void BasicBlock::InsertInstBack(Instruction *inst)
{
    ASSERT(!inst->GetId().IsPhi());
    instructions_.PushBack(inst);
}

void BasicBlock::InsertPhiInst(Instruction *inst)
{
    ASSERT(inst->GetId().IsPhi());
    if (lastPhiInst_ == nullptr) {
        instructions_.PushFront(inst);
    } else {
        inst->LinkBefore(lastPhiInst_->next_);
    }
    lastPhiInst_ = inst;
}

}  // namespace compiler::ir
