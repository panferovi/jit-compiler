#ifndef IR_BUILDER_INL_H
#define IR_BUILDER_INL_H

#include "ir/id.h"
#include "ir/graph.h"
#include "ir/common.h"
#include "ir/basic_block.h"
#include "ir/instruction.h"
#include "ir/ir_builder.h"
#include "utils/macros.h"

#include <utility>

namespace compiler::ir {

template <typename InstType, typename... InstArgs>
InstType *IRBuilder::CreateInstruction(InstArgs... args)
{
    ASSERT(insertionPoint_ != nullptr);
    auto instId = InstId {graph_->NewInstId()};
    auto *inst = new InstType {insertionPoint_, instId, std::forward<InstArgs>(args)...};
    ASSERT(inst->GetOpcode() != Opcode::PHI);
    insertionPoint_->InsertInstBack(inst);
    for (auto *input : inst->GetInputs()) {
        input->AddUsers(inst);
    }
    return inst;
}

PhiInst *IRBuilder::CreatePhiInstruction(ResultType resType)
{
    ASSERT(insertionPoint_ != nullptr);
    auto instId = InstId {graph_->NewInstId(), true};
    auto *inst = new PhiInst {insertionPoint_, instId, resType};
    insertionPoint_->InsertPhiInst(inst);
    return inst;
}

}  // namespace compiler::ir

#endif  // IR_BUILDER_INL_H
