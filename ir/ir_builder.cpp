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

ArithmInst *IRBuilder::CreateShl(Instruction *op1, Instruction *op2)
{
    ASSERT(IsArithmeticOperands(op1, op2));
    return CreateInstruction<ArithmInst>(Opcode::SHL, CombineResultType(op1, op2), InstProxyList {op1, op2});
}

ArithmInst *IRBuilder::CreateXor(Instruction *op1, Instruction *op2)
{
    ASSERT(IsArithmeticOperands(op1, op2));
    return CreateInstruction<ArithmInst>(Opcode::XOR, CombineResultType(op1, op2), InstProxyList {op1, op2});
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
    ASSERT(retValue->GetResultType() != ResultType::VOID);
    return CreateInstruction<ReturnInst>(retValue->GetResultType(), InstProxyList {retValue});
}

ReturnInst *IRBuilder::CreateRetVoid()
{
    return CreateInstruction<ReturnInst>(ResultType::VOID, InstProxyList {});
}

PhiInst *IRBuilder::CreatePhi(ResultType resType)
{
    return CreatePhiInstruction(resType);
}

MemoryInst *IRBuilder::CreateMemory(ResultType resType, Instruction *count)
{
    ASSERT(count->GetResultType() != ResultType::VOID);
    return CreateInstruction<MemoryInst>(resType, InstProxyList {count});
}

LoadInst *IRBuilder::CreateLoad(MemoryInst *mem, Instruction *idx)
{
    ASSERT(idx->GetResultType() != ResultType::VOID);
    return CreateInstruction<LoadInst>(mem->GetResultType(), InstProxyList {mem, idx});
}

StoreInst *IRBuilder::CreateStore(MemoryInst *mem, Instruction *idx, Instruction *value)
{
    ASSERT(value->GetResultType() != ResultType::VOID);
    ASSERT(idx->GetResultType() != ResultType::VOID);
    ASSERT(value->GetResultType() <= mem->GetResultType());
    return CreateInstruction<StoreInst>(InstProxyList {mem, idx, value});
}

CheckInst *IRBuilder::CreateNullCheck(MemoryInst *mem)
{
    return CreateInstruction<CheckInst>(CheckType::NIL, InstProxyList {mem});
}

CheckInst *IRBuilder::CreateBoundCheck(MemoryInst *mem, Instruction *idx)
{
    ASSERT(idx->GetResultType() != ResultType::VOID);
    return CreateInstruction<CheckInst>(CheckType::BOUND, InstProxyList {mem, idx});
}

}  // namespace compiler::ir
