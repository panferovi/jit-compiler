#ifndef IR_IR_BUILDER_H
#define IR_IR_BUILDER_H

#include "ir/common.h"
#include "utils/macros.h"

#include <initializer_list>
#include <cstdint>

namespace compiler::ir {

class Graph;
class BasicBlock;
class Instruction;
class AssignInst;
class ArithmInst;
class LogicInst;
class BranchInst;
class ReturnInst;
class PhiInst;

class IRBuilder {
public:
    IRBuilder(Graph *graph) : graph_(graph) {}

    void SetInsertionPoint(BasicBlock *bb)
    {
        insertionPoint_ = bb;
    }

    AssignInst *CreateConstInt(int value);
    AssignInst *CreateParam(ResultType type, uint32_t id);

    ArithmInst *CreateAdd(Instruction *op1, Instruction *op2);
    ArithmInst *CreateMul(Instruction *op1, Instruction *op2);

    LogicInst *CreateCmpLE(Instruction *op1, Instruction *op2);
    LogicInst *CreateCmpLT(Instruction *op1, Instruction *op2);

    BranchInst *CreateBr(BasicBlock *bb);
    BranchInst *CreateCondBr(Instruction *pred, BasicBlock *trueBr, BasicBlock *falseBr);

    ReturnInst *CreateRet(Instruction *retValue);
    ReturnInst *CreateRetVoid();

    PhiInst *CreatePhi(ResultType resType);

private:
    using InstProxyList = std::initializer_list<Instruction *>;

    template <typename InstType, typename... InstArgs>
    InstType *CreateInstruction(InstArgs... instArgs);

    inline PhiInst *CreatePhiInstruction(ResultType resType);

    Graph *graph_;
    BasicBlock *insertionPoint_ = nullptr;
};

}  // namespace compiler::ir

#endif  // IR_IR_BUILDER_H
