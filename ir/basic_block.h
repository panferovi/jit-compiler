#ifndef IR_BASIC_BLOCK_H
#define IR_BASIC_BLOCK_H

#include "ir/id.h"
#include "ir/marker.h"
#include "utils/intrusive_list.h"
#include "utils/macros.h"

#include <cstdint>
#include <list>
#include <vector>
#include <deque>
#include <functional>

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
    using InterruptibleVisitor = std::function<bool(Instruction *)>;

    Id GetId() const
    {
        return id_;
    }

    Graph *GetGraph()
    {
        return graph_;
    }

    void AddPredeccessor(BasicBlock *bb)
    {
        predecessors_.push_back(bb);
    }

    void SetTrueSuccessor(BasicBlock *bb)
    {
        ASSERT(trueSuccessor_ == nullptr);
        ASSERT(bb != nullptr);
        ASSERT(bb != falseSuccessor_);
        trueSuccessor_ = bb;
        bb->AddPredeccessor(this);
    }

    void SetFalseSuccessor(BasicBlock *bb)
    {
        ASSERT(falseSuccessor_ == nullptr);
        ASSERT(bb != nullptr);
        ASSERT(bb != trueSuccessor_);
        falseSuccessor_ = bb;
        bb->AddPredeccessor(this);
    }

    BasicBlock *GetTrueSuccessor()
    {
        return trueSuccessor_;
    }

    BasicBlock *GetFalseSuccessor()
    {
        return falseSuccessor_;
    }

    void Mark(Marker marker)
    {
        marker_.Mark(marker);
    }

    void Unmark(Marker marker)
    {
        marker_.Unmark(marker);
    }

    bool IsMarked(Marker marker)
    {
        return marker_.IsMarked(marker);
    }

    void SetDfsOrder(uint32_t dfsOrder)
    {
        dfsOrder_ = dfsOrder;
    }

    uint32_t GetDfsOrder() const
    {
        return dfsOrder_;
    }

    std::vector<BasicBlock *> GetSuccessors();

    static BasicBlock *Create(Graph *graph);

    void InsertInstBack(Instruction *inst);

    void InsertPhiInst(Instruction *inst);

    Instruction *GetLastInstruction();

    void Dump(std::stringstream &ss) const;

    void SetDominator(BasicBlock *dominator);

    BasicBlock *GetDominator() const;

    void AddDominatee(BasicBlock *dominatee);

    const std::deque<BasicBlock *> &GetImmediateDominatees() const;

    void IterateOverInstructions(InterruptibleVisitor visitor);

private:
    BasicBlock(Id id, Graph *graph) : id_(id), graph_(graph) {}

    Id id_;
    Graph *graph_;
    utils::IntrusiveList<Instruction> instructions_;
    Predecessors predecessors_;
    BasicBlock *trueSuccessor_ {nullptr};
    BasicBlock *falseSuccessor_ {nullptr};
    Instruction *lastPhiInst_ {nullptr};

    // analysis
    Marker marker_ {};
    uint32_t dfsOrder_ {0};
    BasicBlock *dominator_ {nullptr};
    std::deque<BasicBlock *> immDominatees_;
};

}  // namespace compiler::ir

#endif  // IR_BASIC_BLOCK_H
