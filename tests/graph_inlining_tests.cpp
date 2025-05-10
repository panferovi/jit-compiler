#include <gtest/gtest.h>
#include <sstream>

#include "analysis/optimization.h"
#include "ir/basic_block.h"
#include "ir/call_graph.h"
#include "ir/common.h"
#include "ir/graph.h"
#include "ir/ir_builder.h"
#include "ir/instruction.h"
#include "utils/macros.h"

namespace compiler::tests {

/**
 *   Source Code:
 *       function bar(): int {
 *           const c0 = 1;
 *           const c1 = 7;
 *           let result = c0 << c1;
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Constant 1
 *           1.s32 Constant 7
 *           2. Br BB.1
 *       BB.1:
 *           3.s32 Shl v0, v1
 *           4.s32 Return v3
 *
 *   Source Code:
 *       function foo(): int {
 *           const c0 = 1;
 *           let result = bar() + c0;
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Constant 1
 *           1. Br BB.1
 *       BB.1:
 *           2.s32 CallSt id: 0 Ret: s32
 *           3.s32 Add v2, v0
 *           4.s32 Return v3
 *
 *
 *   IR Graph after inlining:
 *       BB.0:
 *           0.s32 Constant 1
 *           5.s32 Constant 7
 *           1. Br BB.1
 *       BB.1:
 *          10. Br BB.2
 *       BB.2:
 *           6.s32 Shl v0, v5
 *           8. Br BB.3
 *       BB.3:
 *           9p.s32 Phi v6:BB.2
 *           3.s32 Add v9, v0
 *           4.s32 Return v3
 */
TEST(GRAPH_INLINING, SimpleInline)
{
    auto callGraph = ir::CallGraph {};

    auto graphBar = ir::Graph {&callGraph, "bar"};
    {
        auto irBuilder = ir::IRBuilder {&graphBar};

        auto *bb0 = ir::BasicBlock::Create(&graphBar);
        auto *bb1 = ir::BasicBlock::Create(&graphBar);

        irBuilder.SetInsertionPoint(bb0);
        auto *v0 = irBuilder.CreateConstInt(1);
        auto *v1 = irBuilder.CreateConstInt(7);
        [[maybe_unused]] auto *v2 = irBuilder.CreateBr(bb1);

        irBuilder.SetInsertionPoint(bb1);
        auto *v3 = irBuilder.CreateShl(v0, v1);
        [[maybe_unused]] auto *v4 = irBuilder.CreateRet(v3);
    }

    auto graphFoo = ir::Graph {&callGraph, "foo"};
    auto irBuilder = ir::IRBuilder {&graphFoo};

    auto *bb0 = ir::BasicBlock::Create(&graphFoo);
    auto *bb1 = ir::BasicBlock::Create(&graphFoo);

    irBuilder.SetInsertionPoint(bb0);
    auto *v0 = irBuilder.CreateConstInt(1);
    [[maybe_unused]] auto *v1 = irBuilder.CreateBr(bb1);

    irBuilder.SetInsertionPoint(bb1);
    auto *v2 = irBuilder.CreateCallStatic(graphBar.GetMethodId(), ir::ResultType::S32, ir::InstProxyList {});
    auto *v3 = irBuilder.CreateAdd(v2, v0);
    [[maybe_unused]] auto *v4 = irBuilder.CreateRet(v3);

    InliningOptimizer inliningOpt(&graphFoo);
    inliningOpt.Run();

    graphFoo.IterateOverBlocks([](ir::BasicBlock *bb) {
        bb->IterateOverInstructions([bb](ir::Instruction *inst) {
            ASSERT(inst->GetBasicBlock() == bb);
            ASSERT(inst->GetOpcode() != ir::Opcode::CALL_STATIC);
            return false;
        });
    });

    auto *constBB = graphFoo.GetStartBlock();
    ASSERT(constBB == bb0);
    ASSERT(constBB->GetTrueSuccessor() == bb1);
    ASSERT(bb1->GetPredecessors() == ir::BasicBlock::Predecessors {bb0});
    ASSERT(bb1->GetLastInstruction()->GetOpcode() == ir::Opcode::BRANCH);
    auto *bb2 = bb1->GetTrueSuccessor();
    ASSERT(bb2->GetPredecessors() == ir::BasicBlock::Predecessors {bb1});
    ASSERT(bb2->GetLastInstruction()->GetOpcode() == ir::Opcode::BRANCH);
    auto *bb3 = bb2->GetTrueSuccessor();
    ASSERT(bb3->GetPredecessors() == ir::BasicBlock::Predecessors {bb2});
    ASSERT(bb3->GetLastInstruction()->GetOpcode() == ir::Opcode::RETURN);
}

/**
 *   Source Code:
 *       function bar(value: int): int {
 *           const c1 = 7;
 *           let result = value << c1;
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1.s32 Constant 7
 *           2. Br BB.1
 *       BB.1:
 *           3.s32 Shl v0, v1
 *           4.s32 Return v3
 *
 *   Source Code:
 *       function foo(value: int): int {
 *           const c1 = 1;
 *           let result = bar(bar(value)) + c1;
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1.s32 Constant 1
 *           2. Br BB.1
 *       BB.1:
 *           3.s32 CallSt id: 0 Ret: s32 v0
 *           4.s32 CallSt id: 0 Ret: s32 v3
 *           5.s32 Add v4, v1
 *           6.s32 Return v5
 *
 *
 *   IR Graph after inlining:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1.s32 Constant 1
 *           7.s32 Constant 7
 *           2. Br BB.1
 *       BB.1:
 *          12. Br BB.2
 *       BB.2:
 *           8.s32 Shl v0, v7
 *          10. Br BB.3
 *       BB.3:
 *          11p.s32 Phi v8:BB.2
 *          17. Br BB.4
 *       BB.4:
 *          13.s32 Shl v11, v7
 *          15. Br BB.5
 *       BB.5:
 *          16p.s32 Phi v13:BB.4
 *           5.s32 Add v16, v1
 *           6.s32 Return v5
 */
TEST(GRAPH_INLINING, InlineWithDependencies)
{
    auto callGraph = ir::CallGraph {};

    auto graphBar = ir::Graph {&callGraph, "bar"};
    {
        auto irBuilder = ir::IRBuilder {&graphBar};

        auto *bb0 = ir::BasicBlock::Create(&graphBar);
        auto *bb1 = ir::BasicBlock::Create(&graphBar);

        irBuilder.SetInsertionPoint(bb0);
        auto *v0 = irBuilder.CreateParam(ir::ResultType::S32, 0);
        auto *v1 = irBuilder.CreateConstInt(7);
        [[maybe_unused]] auto *v2 = irBuilder.CreateBr(bb1);

        irBuilder.SetInsertionPoint(bb1);
        auto *v3 = irBuilder.CreateShl(v0, v1);
        [[maybe_unused]] auto *v4 = irBuilder.CreateRet(v3);
    }

    auto graphFoo = ir::Graph {&callGraph, "foo"};
    auto irBuilder = ir::IRBuilder {&graphFoo};

    auto *bb0 = ir::BasicBlock::Create(&graphFoo);
    auto *bb1 = ir::BasicBlock::Create(&graphFoo);

    irBuilder.SetInsertionPoint(bb0);
    auto *v0 = irBuilder.CreateParam(ir::ResultType::S32, 0);
    auto *v1 = irBuilder.CreateConstInt(1);
    [[maybe_unused]] auto *v2 = irBuilder.CreateBr(bb1);

    irBuilder.SetInsertionPoint(bb1);
    auto *v3 = irBuilder.CreateCallStatic(graphBar.GetMethodId(), ir::ResultType::S32, ir::InstProxyList {v0});
    auto *v4 = irBuilder.CreateCallStatic(graphBar.GetMethodId(), ir::ResultType::S32, ir::InstProxyList {v3});
    auto *v5 = irBuilder.CreateAdd(v4, v1);
    [[maybe_unused]] auto *v6 = irBuilder.CreateRet(v5);

    InliningOptimizer inliningOpt(&graphFoo);
    inliningOpt.Run();

    graphFoo.IterateOverBlocks([](ir::BasicBlock *bb) {
        bb->IterateOverInstructions([bb](ir::Instruction *inst) {
            ASSERT(inst->GetBasicBlock() == bb);
            ASSERT(inst->GetOpcode() != ir::Opcode::CALL_STATIC);
            return false;
        });
    });

    auto *constBB = graphFoo.GetStartBlock();
    ASSERT(constBB == bb0);
    ASSERT(constBB->GetTrueSuccessor() == bb1);
    ASSERT(bb1->GetPredecessors() == ir::BasicBlock::Predecessors {bb0});
    ASSERT(bb1->GetLastInstruction()->GetOpcode() == ir::Opcode::BRANCH);
    auto *bb2 = bb1->GetTrueSuccessor();
    ASSERT(bb2->GetPredecessors() == ir::BasicBlock::Predecessors {bb1});
    ASSERT(bb2->GetLastInstruction()->GetOpcode() == ir::Opcode::BRANCH);
    auto *bb3 = bb2->GetTrueSuccessor();
    ASSERT(bb3->GetPredecessors() == ir::BasicBlock::Predecessors {bb2});
    ASSERT(bb2->GetLastInstruction()->GetOpcode() == ir::Opcode::BRANCH);
    auto *bb4 = bb3->GetTrueSuccessor();
    ASSERT(bb4->GetPredecessors() == ir::BasicBlock::Predecessors {bb3});
    ASSERT(bb4->GetLastInstruction()->GetOpcode() == ir::Opcode::BRANCH);
    auto *bb5 = bb4->GetTrueSuccessor();
    ASSERT(bb5->GetPredecessors() == ir::BasicBlock::Predecessors {bb4});
    ASSERT(bb5->GetLastInstruction()->GetOpcode() == ir::Opcode::RETURN);
}

/**
 *   Source Code:
 *       function baz(value: int): int {
 *           const c1 = 63;
 *           let result = value ^ c1;
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1.s32 Constant 63
 *           2. Br BB.1
 *       BB.1:
 *           3.s32 Xor v0, v1
 *           4.s32 Return v3
 *
 *   Source Code:
 *       function bar(value: int): int {
 *           const c1 = 7;
 *           let result = baz(value) << c1;
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1.s32 Constant 7
 *           2. Br BB.1
 *       BB.1:
 *           3.s32 CallSt id: 0 Ret: s32 v0
 *           4.s32 Shl v3, v1
 *           5.s32 Return v4
 *
 *   Source Code:
 *       function foo(value: int): int {
 *           const c1 = 1;
 *           let result = bar(value) + c1;
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1.s32 Constant 1
 *           2. Br BB.1
 *       BB.1:
 *           3.s32 CallSt id: 1 Ret: s32 v0
 *           4.s32 Add v3, v1
 *           5.s32 Return v4
 *
 *
 *   IR Graph after inlining:
 *       BB.0:
 *           0.s32 Parameter 0
 *           1.s32 Constant 1
 *           6.s32 Constant 7
 *          13.s32 Constant 63
 *           2. Br BB.1
 *       BB.1:
 *          12. Br BB.2
 *       BB.2:
 *          18. Br BB.4
 *       BB.3:
 *          11p.s32 Phi v8:BB.5
 *           4.s32 Add v11, v1
 *           5.s32 Return v4
 *       BB.4:
 *          14.s32 Xor v0, v13
 *          16. Br BB.5
 *       BB.5:
 *          17p.s32 Phi v14:BB.4
 *           8.s32 Shl v17, v6
 *          10. Br BB.3
 */
TEST(GRAPH_INLINING, ChainInlining)
{
    auto callGraph = ir::CallGraph {};

    auto graphBaz = ir::Graph {&callGraph, "baz"};
    {
        auto irBuilder = ir::IRBuilder {&graphBaz};

        auto *bb0 = ir::BasicBlock::Create(&graphBaz);
        auto *bb1 = ir::BasicBlock::Create(&graphBaz);

        irBuilder.SetInsertionPoint(bb0);
        auto *v0 = irBuilder.CreateParam(ir::ResultType::S32, 0);
        auto *v1 = irBuilder.CreateConstInt(63);
        [[maybe_unused]] auto *v2 = irBuilder.CreateBr(bb1);

        irBuilder.SetInsertionPoint(bb1);
        auto *v3 = irBuilder.CreateXor(v0, v1);
        [[maybe_unused]] auto *v4 = irBuilder.CreateRet(v3);
    }

    auto graphBar = ir::Graph {&callGraph, "bar"};
    {
        auto irBuilder = ir::IRBuilder {&graphBar};

        auto *bb0 = ir::BasicBlock::Create(&graphBar);
        auto *bb1 = ir::BasicBlock::Create(&graphBar);

        irBuilder.SetInsertionPoint(bb0);
        auto *v0 = irBuilder.CreateParam(ir::ResultType::S32, 0);
        auto *v1 = irBuilder.CreateConstInt(7);
        [[maybe_unused]] auto *v2 = irBuilder.CreateBr(bb1);

        irBuilder.SetInsertionPoint(bb1);
        auto *v4 = irBuilder.CreateCallStatic(graphBaz.GetMethodId(), ir::ResultType::S32, ir::InstProxyList {v0});
        auto *v5 = irBuilder.CreateShl(v4, v1);
        [[maybe_unused]] auto *v6 = irBuilder.CreateRet(v5);
    }

    auto graphFoo = ir::Graph {&callGraph, "foo"};
    auto irBuilder = ir::IRBuilder {&graphFoo};

    auto *bb0 = ir::BasicBlock::Create(&graphFoo);
    auto *bb1 = ir::BasicBlock::Create(&graphFoo);

    irBuilder.SetInsertionPoint(bb0);
    auto *v0 = irBuilder.CreateParam(ir::ResultType::S32, 0);
    auto *v1 = irBuilder.CreateConstInt(1);
    [[maybe_unused]] auto *v2 = irBuilder.CreateBr(bb1);

    irBuilder.SetInsertionPoint(bb1);
    auto *v3 = irBuilder.CreateCallStatic(graphBar.GetMethodId(), ir::ResultType::S32, ir::InstProxyList {v0});
    auto *v4 = irBuilder.CreateAdd(v3, v1);
    [[maybe_unused]] auto *v5 = irBuilder.CreateRet(v4);

    InliningOptimizer inliningOpt(&graphFoo);
    inliningOpt.Run();

    graphFoo.IterateOverBlocks([](ir::BasicBlock *bb) {
        bb->IterateOverInstructions([bb](ir::Instruction *inst) {
            ASSERT(inst->GetBasicBlock() == bb);
            ASSERT(inst->GetOpcode() != ir::Opcode::CALL_STATIC);
            return false;
        });
    });

    auto *constBB = graphFoo.GetStartBlock();
    ASSERT(constBB == bb0);
    ASSERT(constBB->GetTrueSuccessor() == bb1);
    ASSERT(bb1->GetPredecessors() == ir::BasicBlock::Predecessors {bb0});
    ASSERT(bb1->GetLastInstruction()->GetOpcode() == ir::Opcode::BRANCH);
    auto *bb2 = bb1->GetTrueSuccessor();
    ASSERT(bb2->GetPredecessors() == ir::BasicBlock::Predecessors {bb1});
    ASSERT(bb2->GetLastInstruction()->GetOpcode() == ir::Opcode::BRANCH);
    auto *bb3 = bb2->GetTrueSuccessor();
    ASSERT(bb3->GetPredecessors() == ir::BasicBlock::Predecessors {bb2});
    ASSERT(bb2->GetLastInstruction()->GetOpcode() == ir::Opcode::BRANCH);
    auto *bb4 = bb3->GetTrueSuccessor();
    ASSERT(bb4->GetPredecessors() == ir::BasicBlock::Predecessors {bb3});
    ASSERT(bb4->GetLastInstruction()->GetOpcode() == ir::Opcode::BRANCH);
    auto *bb5 = bb4->GetTrueSuccessor();
    ASSERT(bb5->GetPredecessors() == ir::BasicBlock::Predecessors {bb4});
    ASSERT(bb5->GetLastInstruction()->GetOpcode() == ir::Opcode::RETURN);
}

}  // namespace compiler::tests
