#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include "analysis/optimization.h"
#include "ir/basic_block.h"
#include "ir/common.h"
#include "ir/graph.h"
#include "ir/ir_builder.h"
#include "ir/instruction.h"
#include "utils/macros.h"

namespace compiler::tests {

/**
 *   Source Code:
 *       function foo(value: int): int {
 *           const c1 = 0;
 *           let result = value + c1;
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1.s32 Constant 0
 *           2. Br BB.1
 *       BB.1:
 *           3.s32 Add v0, v1
 *           4.s32 Return v3
 *
 *   After peephole optimizer:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1.s32 Constant 0
 *           2. Br BB.1
 *       BB.1:
 *           4.s32 Return v0
 */
TEST(PEEPHOLE_OPT, AddPeepholeOneOpZero)
{
    auto graph = ir::Graph {};
    auto irBuilder = ir::IRBuilder {&graph};

    auto *bb0 = ir::BasicBlock::Create(&graph);
    auto *bb1 = ir::BasicBlock::Create(&graph);

    irBuilder.SetInsertionPoint(bb0);
    auto *v0 = irBuilder.CreateParam(ir::ResultType::S32, 0);
    auto *v1 = irBuilder.CreateConstInt(0);
    [[maybe_unused]] auto *v2 = irBuilder.CreateBr(bb1);

    irBuilder.SetInsertionPoint(bb1);
    auto *v3 = irBuilder.CreateAdd(v0, v1);
    auto *v4 = irBuilder.CreateRet(v3);

    PeepHoleOptimizer peepHoleOpt(&graph);
    peepHoleOpt.Run();

    ASSERT(bb1->GetAliveInstructionCount() == 1);

    ASSERT(v4->GetFirstOp() == v0);
    ASSERT(v0->GetUsers() == ir::Instruction::Users {v4});
}

/**
 *   Source Code:
 *       function foo(value: int): int {
 *           let result = value + value;
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1. Br BB.1
 *       BB.1:
 *           2.s32 Add v0, v0
 *           3.s32 Return v2
 *
 *   After peephole optimizer:
 *       BB.0:
 *           0.s32 Parameter 0
 *           4.u8 Constant 1
 *           1. Br BB.1
 *       BB.1:
 *           2.s32 Shl v0, v4
 *           3.s32 Return v2
 */
TEST(PEEPHOLE_OPT, AddPeepholeSameValue)
{
    auto graph = ir::Graph {};
    auto irBuilder = ir::IRBuilder {&graph};

    auto *bb0 = ir::BasicBlock::Create(&graph);
    auto *bb1 = ir::BasicBlock::Create(&graph);

    irBuilder.SetInsertionPoint(bb0);
    auto *v0 = irBuilder.CreateParam(ir::ResultType::S32, 0);
    [[maybe_unused]] auto *v1 = irBuilder.CreateBr(bb1);

    irBuilder.SetInsertionPoint(bb1);
    auto *v2 = irBuilder.CreateAdd(v0, v0);
    auto *v3 = irBuilder.CreateRet(v2);

    PeepHoleOptimizer peepHoleOpt(&graph);
    peepHoleOpt.Run();

    ASSERT(bb1->GetAliveInstructionCount() == 2);

    auto *shlInst = v3->GetFirstOp();
    auto *constOne = shlInst->GetSecondOp();
    ASSERT(shlInst->GetUsers() == ir::Instruction::Users {v3});
    ASSERT(shlInst->GetOpcode() == ir::Opcode::SHL);
    ASSERT(shlInst->GetFirstOp() == v0);
    ASSERT(constOne->GetOpcode() == ir::Opcode::CONSTANT);
    ASSERT(constOne->As<ir::AssignInst>()->GetValue() == 1);
}

/**
 *   Source Code:
 *       function foo(value: unsigned): int {
 *           const c1 = 0;
 *           let result = c1 << value;
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1.s32 Constant 0
 *           2. Br BB.1
 *       BB.1:
 *           3.s32 Shl v1, v0
 *           4.s32 Return v3
 *
 *   After peephole optimizer:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1.s32 Constant 0
 *           2. Br BB.1
 *       BB.1:
 *           4.s32 Return v1
 */
TEST(PEEPHOLE_OPT, ShlPeepholeFirstOpZero)
{
    auto graph = ir::Graph {};
    auto irBuilder = ir::IRBuilder {&graph};

    auto *bb0 = ir::BasicBlock::Create(&graph);
    auto *bb1 = ir::BasicBlock::Create(&graph);

    irBuilder.SetInsertionPoint(bb0);
    auto *v0 = irBuilder.CreateParam(ir::ResultType::S32, 0);
    auto *v1 = irBuilder.CreateConstInt(0);
    [[maybe_unused]] auto *v2 = irBuilder.CreateBr(bb1);

    irBuilder.SetInsertionPoint(bb1);
    auto *v3 = irBuilder.CreateShl(v1, v0);
    auto *v4 = irBuilder.CreateRet(v3);

    PeepHoleOptimizer peepHoleOpt(&graph);
    peepHoleOpt.Run();

    ASSERT(bb1->GetAliveInstructionCount() == 1);

    ASSERT(v4->GetFirstOp() == v1);
    ASSERT(v1->GetUsers() == ir::Instruction::Users {v4});
}

/**
 *   Source Code:
 *       function foo(value: unsigned): int {
 *           const c1 = 0;
 *           let result = value << c1;
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1.s32 Constant 0
 *           2. Br BB.1
 *       BB.1:
 *           3.s32 Shl v0, v1
 *           4.s32 Return v3
 *
 *   After peephole optimizer:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1.s32 Constant 0
 *           2. Br BB.1
 *       BB.1:
 *           4.s32 Return v0
 */
TEST(PEEPHOLE_OPT, ShlPeepholeSecondOpZero)
{
    auto graph = ir::Graph {};
    auto irBuilder = ir::IRBuilder {&graph};

    auto *bb0 = ir::BasicBlock::Create(&graph);
    auto *bb1 = ir::BasicBlock::Create(&graph);

    irBuilder.SetInsertionPoint(bb0);
    auto *v0 = irBuilder.CreateParam(ir::ResultType::S32, 0);
    auto *v1 = irBuilder.CreateConstInt(0);
    [[maybe_unused]] auto *v2 = irBuilder.CreateBr(bb1);

    irBuilder.SetInsertionPoint(bb1);
    auto *v3 = irBuilder.CreateShl(v0, v1);
    auto *v4 = irBuilder.CreateRet(v3);

    PeepHoleOptimizer peepHoleOpt(&graph);
    peepHoleOpt.Run();

    ASSERT(bb1->GetAliveInstructionCount() == 1);

    ASSERT(v4->GetFirstOp() == v0);
    ASSERT(v0->GetUsers() == ir::Instruction::Users {v4});
}

/**
 *   Source Code:
 *       function foo(value: int): int {
 *           const c1 = 0;
 *           let result = value + c1;
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1.s32 Constant 0
 *           2. Br BB.1
 *       BB.1:
 *           3.s32 Xor v0, v1
 *           4.s32 Return v3
 *
 *   After peephole optimizer:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1.s32 Constant 0
 *           2. Br BB.1
 *       BB.1:
 *           4.s32 Return v0
 */
TEST(PEEPHOLE_OPT, XorPeepholeOneOpZero)
{
    auto graph = ir::Graph {};
    auto irBuilder = ir::IRBuilder {&graph};

    auto *bb0 = ir::BasicBlock::Create(&graph);
    auto *bb1 = ir::BasicBlock::Create(&graph);

    irBuilder.SetInsertionPoint(bb0);
    auto *v0 = irBuilder.CreateParam(ir::ResultType::S32, 0);
    auto *v1 = irBuilder.CreateConstInt(0);
    [[maybe_unused]] auto *v2 = irBuilder.CreateBr(bb1);

    irBuilder.SetInsertionPoint(bb1);
    auto *v3 = irBuilder.CreateXor(v0, v1);
    auto *v4 = irBuilder.CreateRet(v3);

    PeepHoleOptimizer peepHoleOpt(&graph);
    peepHoleOpt.Run();

    ASSERT(bb1->GetAliveInstructionCount() == 1);

    ASSERT(v4->GetFirstOp() == v0);
    ASSERT(v0->GetUsers() == ir::Instruction::Users {v4});
}

/**
 *   Source Code:
 *       function foo(value: int): int {
 *           let result = value + value;
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1. Br BB.1
 *       BB.1:
 *           2.s32 Xor v0, v0
 *           3.s32 Return v2
 *
 *   After peephole optimizer:
 *       BB.0:
 *           0.s32 Parameter 0
 *           4.u8 Constant 0
 *           1. Br BB.1
 *       BB.1:
 *           3.s32 Return v4
 */
TEST(PEEPHOLE_OPT, XorPeepholeSameValue)
{
    auto graph = ir::Graph {};
    auto irBuilder = ir::IRBuilder {&graph};

    auto *bb0 = ir::BasicBlock::Create(&graph);
    auto *bb1 = ir::BasicBlock::Create(&graph);

    irBuilder.SetInsertionPoint(bb0);
    auto *v0 = irBuilder.CreateParam(ir::ResultType::S32, 0);
    [[maybe_unused]] auto *v1 = irBuilder.CreateBr(bb1);

    irBuilder.SetInsertionPoint(bb1);
    auto *v2 = irBuilder.CreateXor(v0, v0);
    auto *v3 = irBuilder.CreateRet(v2);

    PeepHoleOptimizer peepHoleOpt(&graph);
    peepHoleOpt.Run();

    ASSERT(bb1->GetAliveInstructionCount() == 1);

    auto *constZero = v3->GetFirstOp();
    ASSERT(constZero->GetUsers() == ir::Instruction::Users {v3});
    ASSERT(constZero->GetOpcode() == ir::Opcode::CONSTANT);
    ASSERT(constZero->As<ir::AssignInst>()->GetValue() == 0);
}

/**
 *   Source Code:
 *       function foo(): int {
 *           const c0 = 1;
 *           const c1 = 4;
 *           const c2 = 6;
 *
 *           let result = ((c2 ^ c1) << c0) + c1;
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Constant 1
 *           1.s32 Constant 4
 *           2.s32 Constant 6
 *           3. Br BB.1
 *       BB.1:
 *           4.s32 Xor v2, v1
 *           5.s32 Shl v4, v0
 *           6.s32 Add v5, v1
 *           7.s32 Return v6
 *
 *   After peephole optimizer:
 *       BB.0:
 *           0.s32 Constant 1
 *           1.s32 Constant 4
 *           2.s32 Constant 6
 *           8.s32 Constant 2
 *           9.s32 Constant 8
 *           3. Br BB.1
 *       BB.1:
 *           7.s32 Return v9
 */
TEST(PEEPHOLE_OPT, ConstFolding)
{
    auto graph = ir::Graph {};
    auto irBuilder = ir::IRBuilder {&graph};

    auto *bb0 = ir::BasicBlock::Create(&graph);
    auto *bb1 = ir::BasicBlock::Create(&graph);

    irBuilder.SetInsertionPoint(bb0);
    auto *v0 = irBuilder.CreateConstInt(1);
    auto *v1 = irBuilder.CreateConstInt(4);
    auto *v2 = irBuilder.CreateConstInt(6);
    [[maybe_unused]] auto *v3 = irBuilder.CreateBr(bb1);

    irBuilder.SetInsertionPoint(bb1);
    auto *v4 = irBuilder.CreateXor(v2, v1);
    auto *v5 = irBuilder.CreateShl(v4, v0);
    auto *v6 = irBuilder.CreateAdd(v5, v1);
    auto *v7 = irBuilder.CreateRet(v6);

    PeepHoleOptimizer peepHoleOpt(&graph);
    peepHoleOpt.Run();

    ASSERT(bb1->GetAliveInstructionCount() == 1);

    auto *constEight = v7->GetFirstOp();
    ASSERT(constEight->GetUsers() == ir::Instruction::Users {v7});
    ASSERT(constEight->GetOpcode() == ir::Opcode::CONSTANT);
    ASSERT(constEight->As<ir::AssignInst>()->GetValue() == 8);
}

/**
 *   Source Code:
 *       function foo(flag: boolean): int {
 *           const c1 = 0;
 *           const c2 = 1;
 *           const c3 = 2;
 *
 *           let result = c1;
 *           if (flag)
 *               result = (c2 << c2) ^ c3;
 *           }
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.b Parameter 0
 *           1.s32 Constant 0
 *           2.s32 Constant 1
 *           3.s32 Constant 2
 *           4. Br BB.1
 *       BB.1:
 *           5. If v0, BB.2, BB.3
 *       BB.2:
 *           6.s32 Shl v2, v2
 *           7.s32 Xor v3, v6
 *           8. Br BB.3
 *       BB.3:
 *           9p.s32 Phi v1:BB.1, v7:BB.2
 *           10.s32 Return v9
 *
 *   After peephole optimizer:
 *       BB.0:
 *           0.b Parameter 0
 *           1.s32 Constant 0
 *           2.s32 Constant 1
 *           3.s32 Constant 2
 *           4. Br BB.1
 *       BB.1:
 *           5. If v0, BB.2, BB.3
 *       BB.2:
 *           8. Br BB.3
 *       BB.3:
 *           10.s32 Return v1
 *
 */
TEST(PEEPHOLE_OPT, ConstFoldingWithPhi)
{
    auto graph = ir::Graph {};
    auto irBuilder = ir::IRBuilder {&graph};

    auto *bb0 = ir::BasicBlock::Create(&graph);
    auto *bb1 = ir::BasicBlock::Create(&graph);
    auto *bb2 = ir::BasicBlock::Create(&graph);
    auto *bb3 = ir::BasicBlock::Create(&graph);

    irBuilder.SetInsertionPoint(bb0);
    auto *v0 = irBuilder.CreateParam(ir::ResultType::BOOL, 0);
    auto *v1 = irBuilder.CreateConstInt(0);
    auto *v2 = irBuilder.CreateConstInt(1);
    auto *v3 = irBuilder.CreateConstInt(2);
    [[maybe_unused]] auto *v4 = irBuilder.CreateBr(bb1);

    irBuilder.SetInsertionPoint(bb1);
    [[maybe_unused]] auto *v5 = irBuilder.CreateCondBr(v0, bb2, bb3);

    irBuilder.SetInsertionPoint(bb2);
    auto *v6 = irBuilder.CreateShl(v2, v2);
    auto *v7 = irBuilder.CreateXor(v3, v6);
    [[maybe_unused]] auto *v8 = irBuilder.CreateBr(bb3);

    irBuilder.SetInsertionPoint(bb3);
    auto *v9 = irBuilder.CreatePhi(ir::ResultType::S32);
    [[maybe_unused]] auto *v10 = irBuilder.CreateRet(v9);

    v9->ResolveDependency(v1, bb1);
    v9->ResolveDependency(v7, bb2);

    PeepHoleOptimizer peepHoleOpt(&graph);
    peepHoleOpt.Run();

    ASSERT(bb2->GetAliveInstructionCount() == 1);
    ASSERT(bb3->GetAliveInstructionCount() == 1);

    ASSERT(v1->GetUsers() == ir::Instruction::Users {v10})
    ASSERT(v10->GetInputs() == ir::Instruction::Inputs {v1});
}

}  // namespace compiler::tests
