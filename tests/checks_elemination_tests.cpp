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
 *       function foo(): unsigned {
 *           const c0 = 0;
 *           const c1 = 1;
 *           const c2 = 10;
 *           let mem = new unsigned[c2];
 *           mem[c0] = c0;
 *           mem[c1] = c0;
 *           mem[c0] = c2;
 *           let result = mem[c1];
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Constant 0
 *           1.s32 Constant 1
 *           2.s32 Constant 10
 *           3. Br BB.1
 *       BB.1:
 *           4.u32 Mem v2
 *           5. Check Nil v4
 *           6. Check Bound v4, v0
 *           7. Store v4, v0, v0
 *           8. Check Bound v4, v1
 *           9. Store v4, v1, v0
 *          10. Check Bound v4, v0
 *          11. Store v4, v0, v0
 *          12. Check Nil v4
 *          13. Check Bound v4, v1
 *          14.u32 Load v4, v1
 *          15.u32 Return v14
 *
 *   After checks optimizer:
 *       BB.0:
 *           0.s32 Constant 0
 *           1.s32 Constant 1
 *           2.s32 Constant 10
 *           3. Br BB.1
 *       BB.1:
 *           4.u32 Mem v2
 *           5. Check Nil v4
 *           6. Check Bound v4, v0
 *           7. Store v4, v0, v0
 *           8. Check Bound v4, v1
 *           9. Store v4, v1, v0
 *          11. Store v4, v0, v0
 *          14.u32 Load v4, v1
 *          15.u32 Return v14
 */
TEST(CHECKS_OPT, DominatedChecksElimination)
{
    auto graph = ir::Graph {};
    auto irBuilder = ir::IRBuilder {&graph};

    auto *bb0 = ir::BasicBlock::Create(&graph);
    auto *bb1 = ir::BasicBlock::Create(&graph);

    irBuilder.SetInsertionPoint(bb0);
    auto *v0 = irBuilder.CreateConstInt(0);
    auto *v1 = irBuilder.CreateConstInt(1);
    auto *v2 = irBuilder.CreateConstInt(10);
    [[maybe_unused]] auto *v3 = irBuilder.CreateBr(bb1);

    irBuilder.SetInsertionPoint(bb1);
    auto *v4 = irBuilder.CreateMemory(ir::ResultType::U32, v2);
    auto *v5 = irBuilder.CreateNullCheck(v4);

    auto *v6 = irBuilder.CreateBoundCheck(v4, v0);
    [[maybe_unused]] auto *v7 = irBuilder.CreateStore(v4, v0, v0);

    auto *v8 = irBuilder.CreateBoundCheck(v4, v1);
    [[maybe_unused]] auto *v9 = irBuilder.CreateStore(v4, v1, v0);

    auto *v10 = irBuilder.CreateBoundCheck(v4, v0);
    [[maybe_unused]] auto *v11 = irBuilder.CreateStore(v4, v0, v2);

    auto *v12 = irBuilder.CreateNullCheck(v4);
    auto *v13 = irBuilder.CreateBoundCheck(v4, v1);
    auto *v14 = irBuilder.CreateLoad(v4, v1);

    [[maybe_unused]] auto *v15 = irBuilder.CreateRet(v14);

    CheckOptimizer checksElem(&graph);
    checksElem.Run();

    std::set<ir::Instruction *> eliminatedChecks = {v10, v12, v13};
    std::set<ir::Instruction *> remainingChecks = {v5, v6, v8};

    bb1->IterateOverInstructions([&eliminatedChecks, &remainingChecks](ir::Instruction *inst) {
        if (inst->GetOpcode() == ir::Opcode::CHECK) {
            ASSERT(eliminatedChecks.find(inst) == eliminatedChecks.end());
            ASSERT(remainingChecks.find(inst) != remainingChecks.end());
        }
        return false;
    });
}

/**
 *   Source Code:
 *       function foo(): void {
 *           const c0 = 0;
 *           const c1 = 1;
 *           const c2 = 10;
 *           let mem1 = new unsigned[c2];
 *           let mem2 = new unsigned[c2];
 *           mem1[c1] = c0;
 *           mem2[c1] = c0;
 *           return;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Constant 0
 *           1.s32 Constant 1
 *           2.s32 Constant 10
 *           3. Br BB.1
 *       BB.1:
 *           4.u32 Mem v2
 *           5. Check Nil v4
 *           6.u32 Mem v2
 *           7. Check Nil v6
 *           8. Check Bound v4, v1
 *           9. Store v4, v1, v0
 *          10. Check Bound v6, v1
 *          11. Store v6, v1, v0
 *          12. Return void
 *
 *   After checks optimizer:
 *       BB.0:
 *           0.s32 Constant 0
 *           1.s32 Constant 1
 *           2.s32 Constant 10
 *           3. Br BB.1
 *       BB.1:
 *           4.u32 Mem v2
 *           5. Check Nil v4
 *           6.u32 Mem v2
 *           7. Check Nil v6
 *           8. Check Bound v4, v1
 *           9. Store v4, v1, v0
 *          10. Check Bound v6, v1
 *          11. Store v6, v1, v0
 *          12. Return void
 */
TEST(CHECKS_OPT, DifferentMemoryChecksElemination)
{
    auto graph = ir::Graph {};
    auto irBuilder = ir::IRBuilder {&graph};

    auto *bb0 = ir::BasicBlock::Create(&graph);
    auto *bb1 = ir::BasicBlock::Create(&graph);

    irBuilder.SetInsertionPoint(bb0);
    auto *v0 = irBuilder.CreateConstInt(0);
    auto *v1 = irBuilder.CreateConstInt(1);
    auto *v2 = irBuilder.CreateConstInt(10);
    [[maybe_unused]] auto *v3 = irBuilder.CreateBr(bb1);

    irBuilder.SetInsertionPoint(bb1);
    auto *v4 = irBuilder.CreateMemory(ir::ResultType::U32, v2);
    auto *v5 = irBuilder.CreateNullCheck(v4);

    auto *v6 = irBuilder.CreateMemory(ir::ResultType::U32, v2);
    auto *v7 = irBuilder.CreateNullCheck(v6);

    auto *v8 = irBuilder.CreateBoundCheck(v4, v1);
    [[maybe_unused]] auto *v9 = irBuilder.CreateStore(v4, v1, v0);

    auto *v10 = irBuilder.CreateBoundCheck(v6, v1);
    [[maybe_unused]] auto *v11 = irBuilder.CreateStore(v6, v1, v0);

    [[maybe_unused]] auto *v12 = irBuilder.CreateRetVoid();

    CheckOptimizer checksElem(&graph);
    checksElem.Run();

    std::set<ir::Instruction *> eliminatedChecks = {};
    std::set<ir::Instruction *> remainingChecks = {v5, v7, v8, v10};

    bb1->IterateOverInstructions([&eliminatedChecks, &remainingChecks](ir::Instruction *inst) {
        if (inst->GetOpcode() == ir::Opcode::CHECK) {
            ASSERT(eliminatedChecks.find(inst) == eliminatedChecks.end());
            ASSERT(remainingChecks.find(inst) != remainingChecks.end());
        }
        return false;
    });
}

}  // namespace compiler::tests
