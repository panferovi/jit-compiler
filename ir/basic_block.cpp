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
    auto *bb = new BasicBlock(graph->NewBBId(), graph);
    graph->InsertBasicBlock(bb);
    return bb;
}

Instruction *BasicBlock::GetLastInstruction()
{
    if (instructions_.IsEmpty()) {
        return nullptr;
    }
    return *instructions_.rbegin();
}

std::vector<BasicBlock *> BasicBlock::GetSuccessors()
{
    std::vector<BasicBlock *> succ;
    if (trueSuccessor_) {
        succ.push_back(trueSuccessor_);
    }
    if (falseSuccessor_) {
        succ.push_back(falseSuccessor_);
    }
    return succ;
}

void BasicBlock::Dump(std::stringstream &ss) const
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

void BasicBlock::SetDominator(BasicBlock *dominator)
{
    ASSERT(dominator_ == nullptr);
    dominator_ = dominator;
}

BasicBlock *BasicBlock::GetDominator() const
{
    return dominator_;
}

void BasicBlock::AddDominatee(BasicBlock *dominatee)
{
    immDominatees_.push_back(dominatee);
}

const std::deque<BasicBlock *> &BasicBlock::GetImmediateDominatees() const
{
    return immDominatees_;
}

void BasicBlock::IterateOverInstructions(InterruptibleVisitor visitor)
{
    for (auto instIt = instructions_.begin(); instIt != instructions_.end();) {
        auto next = std::next(instIt);
        if (visitor(*instIt)) {
            return;
        }
        instIt = std::next(instIt) != nullptr ? std::next(instIt) : next;
    }
}

size_t BasicBlock::GetAliveInstructionCount()
{
    return instructions_.Size();
}

}  // namespace compiler::ir
