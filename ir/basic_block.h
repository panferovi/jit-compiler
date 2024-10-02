#ifndef IR_BASIC_BLOCK_H
#define IR_BASIC_BLOCK_H

#include "ir/id.h"
#include "utils/intrusive_list.h"
#include "utils/macros.h"

#include <list>

namespace compiler::ir {

class Instruction;

class BasicBlock : public utils::IntrusiveListNode<BasicBlock> {
public:
    BasicBlock(Id id) : id_(id) {}

    static BasicBlock *Create(Id id);

    using Predecessors = std::list<BasicBlock *>;

    void InsertInstBack(Instruction *inst);

    void InsertPhiInst(Instruction *inst);

    Id GetId() const
    {
        return id_;
    }

    void SetTrueSuccessor(BasicBlock *bb)
    {
        ASSERT(trueSuccessor_ == nullptr);
        ASSERT(bb != nullptr);
        trueSuccessor_ = bb;
    }

    void SetFalseSuccessor(BasicBlock *bb)
    {
        ASSERT(falseSuccessor_ == nullptr);
        ASSERT(bb != nullptr);
        falseSuccessor_ = bb;
    }

private:
    Id id_;
    utils::IntrusiveList<Instruction> instructions_;
    Predecessors predecessors_;
    BasicBlock *trueSuccessor_ {nullptr};
    BasicBlock *falseSuccessor_ {nullptr};
    Instruction *lastPhiInst_ {nullptr};
};

}  // namespace compiler::ir

#endif  // IR_BASIC_BLOCK_H
