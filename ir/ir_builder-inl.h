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
    auto instId = InstId {graph_->GenerateInstId()};
    auto *inst = new InstType {instId, std::forward<InstArgs>(args)...};
    ASSERT(inst->GetOpcode() != Opcode::PHI);
    ASSERT(insertionPoint_ != nullptr);
    insertionPoint_->InsertInstBack(inst);
    return inst;
}

PhiInst *IRBuilder::CreatePhiInstruction(ResultType resType)
{
    auto instId = InstId {graph_->GenerateInstId(), true};
    auto *inst = new PhiInst {instId, resType};
    ASSERT(insertionPoint_ != nullptr);
    insertionPoint_->InsertPhiInst(inst);
    return inst;
}

}  // namespace compiler::ir

#endif  // IR_BUILDER_INL_H
