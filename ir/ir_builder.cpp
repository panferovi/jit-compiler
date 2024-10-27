#include "ir/id.h"
#include "ir/graph.h"
#include "ir/common.h"
#include "ir/basic_block.h"
#include "ir/instruction.h"
#include "ir/ir_builder.h"
#include "ir/ir_builder-inl.h"
#include "utils/macros.h"

namespace compiler::ir {

#ifndef NDEBUG
static bool IsArithmeticOperands(Instruction *op1, Instruction *op2)
{
    return op1->GetResultType() != ResultType::VOID && op2->GetResultType() != ResultType::VOID;
}
#endif

static ResultType CombineResultType(Instruction *op1, Instruction *op2)
{
    auto resType1 = static_cast<int>(op1->GetResultType());
    auto resType2 = static_cast<int>(op2->GetResultType());
    return static_cast<ResultType>(std::max(resType1, resType2));
}

AssignInst *IRBuilder::CreateConstInt(int value)
{
    return CreateInstruction<AssignInst>(Opcode::CONSTANT, ResultType::S32, value);
}

AssignInst *IRBuilder::CreateParam(ResultType type, uint32_t id)
{
    return CreateInstruction<AssignInst>(Opcode::PARAMETER, type, id);
}

ArithmInst *IRBuilder::CreateAdd(Instruction *op1, Instruction *op2)
{
    ASSERT(IsArithmeticOperands(op1, op2));
    return CreateInstruction<ArithmInst>(Opcode::ADD, CombineResultType(op1, op2), InstProxyList {op1, op2});
}

ArithmInst *IRBuilder::CreateMul(Instruction *op1, Instruction *op2)
{
    ASSERT(IsArithmeticOperands(op1, op2));
    return CreateInstruction<ArithmInst>(Opcode::MUL, CombineResultType(op1, op2), InstProxyList {op1, op2});
}

LogicInst *IRBuilder::CreateCmpLE(Instruction *op1, Instruction *op2)
{
    ASSERT(IsArithmeticOperands(op1, op2));
    return CreateInstruction<LogicInst>(Opcode::COMPARE, InstProxyList {op1, op2}, CmpFlags::LE);
}

LogicInst *IRBuilder::CreateCmpLT(Instruction *op1, Instruction *op2)
{
    ASSERT(IsArithmeticOperands(op1, op2));
    return CreateInstruction<LogicInst>(Opcode::COMPARE, InstProxyList {op1, op2}, CmpFlags::LT);
}

BranchInst *IRBuilder::CreateBr(BasicBlock *bb)
{
    ASSERT(insertionPoint_ != nullptr);
    insertionPoint_->SetTrueSuccessor(bb);
    return CreateInstruction<BranchInst>(Opcode::BRANCH, InstProxyList {});
}

BranchInst *IRBuilder::CreateCondBr(Instruction *pred, BasicBlock *trueBr, BasicBlock *falseBr)
{
    ASSERT(insertionPoint_ != nullptr);
    insertionPoint_->SetTrueSuccessor(trueBr);
    insertionPoint_->SetFalseSuccessor(falseBr);
    return CreateInstruction<BranchInst>(Opcode::COND_BRANCH, InstProxyList {pred});
}

ReturnInst *IRBuilder::CreateRet(Instruction *retValue)
{
    return CreateInstruction<ReturnInst>(Opcode::RETURN, retValue->GetResultType(), InstProxyList {retValue});
}

ReturnInst *IRBuilder::CreateRetVoid()
{
    return CreateInstruction<ReturnInst>(Opcode::RETURN, ResultType::VOID, InstProxyList {});
}

PhiInst *IRBuilder::CreatePhi(ResultType resType)
{
    return CreatePhiInstruction(resType);
}

}  // namespace compiler::ir
