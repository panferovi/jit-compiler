#ifndef IR_BASIC_BLOCK_H
#define IR_BASIC_BLOCK_H

#include "ir/id.h"
#include "utils/intrusive_list.h"
#include "utils/macros.h"

#include <list>

namespace compiler::ir {

class Instruction;
class Graph;

class BasicBlock : public utils::IntrusiveListNode<BasicBlock> {
public:
    explicit BasicBlock() = default;
    NO_COPY_SEMANTIC(BasicBlock);
    NO_MOVE_SEMANTIC(BasicBlock);
    ~BasicBlock();

    using Predecessors = std::list<BasicBlock *>;

    Id GetId() const
    {
        return id_;
    }

    void AddPredeccessor(BasicBlock *bb)
    {
        predecessors_.push_back(bb);
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

    BasicBlock *GetTrueSuccessor()
    {
        return trueSuccessor_;
    }

    BasicBlock *GetFalseSuccessor()
    {
        return falseSuccessor_;
    }

    static BasicBlock *Create(Graph *graph);

    void InsertInstBack(Instruction *inst);

    void InsertPhiInst(Instruction *inst);

    Instruction *GetLastInstruction();

    void Dump(std::stringstream &ss);

private:
    BasicBlock(Id id, Graph *graph) : id_(id), graph_(graph) {}

    Id id_;
    Graph *graph_;
    utils::IntrusiveList<Instruction> instructions_;
    Predecessors predecessors_;
    BasicBlock *trueSuccessor_ {nullptr};
    BasicBlock *falseSuccessor_ {nullptr};
    Instruction *lastPhiInst_ {nullptr};
};

}  // namespace compiler::ir

#endif  // IR_BASIC_BLOCK_H
