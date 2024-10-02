#include "ir/basic_block.h"
#include "ir/instruction.h"
#include "ir/graph.h"

#include <iomanip>

namespace compiler::ir {

void BasicBlock::InsertInstBack(Instruction *inst)
{
    ASSERT(!inst->GetInstId().IsPhi());
    instructions_.PushBack(inst);
}

void BasicBlock::InsertPhiInst(Instruction *inst)
{
    ASSERT(inst->GetInstId().IsPhi());
    if (lastPhiInst_ == nullptr) {
        instructions_.PushFront(inst);
    } else {
        inst->LinkBefore(lastPhiInst_->next_);
    }
    lastPhiInst_ = inst;
}

/* static */
BasicBlock *BasicBlock::Create(Graph *graph)
{
    auto *bb = new BasicBlock(graph->GenerateBBId(), graph);
    graph->InsertBasicBlock(bb);
    return bb;
}

Instruction *BasicBlock::GetLastInstruction()
{
    return *instructions_.rbegin();
}

void BasicBlock::Dump(std::stringstream &ss)
{
    ss << "BB." << id_ << ':' << '\n';
    for (auto *inst : instructions_) {
        ss << std::setw(5);
        inst->Dump(ss);
        ss << '\n';
    }
}

BasicBlock::~BasicBlock()
{
    while (instructions_.NonEmpty()) {
        auto *inst = instructions_.PopFront();
        delete inst;
    }
}

}  // namespace compiler::ir
