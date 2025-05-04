#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include "ir/basic_block.h"
#include "ir/common.h"
#include "ir/graph.h"
#include "ir/ir_builder.h"
#include "ir/instruction.h"
#include "utils/macros.h"

namespace compiler::tests {

TEST(PEEPHOLE_OPT, AddPeephole)
{
    auto graph = ir::Graph {};
    auto irBuilder = ir::IRBuilder {&graph};

    auto *bb1 = ir::BasicBlock::Create(&graph);
    auto *bb2 = ir::BasicBlock::Create(&graph);
    auto *bb3 = ir::BasicBlock::Create(&graph);
    auto *bb4 = ir::BasicBlock::Create(&graph);

    irBuilder.SetInsertionPoint(bb1);
    auto *v0 = irBuilder.CreateParam(ir::ResultType::S32, 0);
    auto *v1 = irBuilder.CreateConstInt(1);
    auto *v2 = irBuilder.CreateConstInt(2);
    [[maybe_unused]] auto *v3 = irBuilder.CreateBr(bb2);
}

TEST(PEEPHOLE_OPT, ShlPeephole)
{
    auto graph = ir::Graph {};
    auto irBuilder = ir::IRBuilder {&graph};

    auto *bb1 = ir::BasicBlock::Create(&graph);
    auto *bb2 = ir::BasicBlock::Create(&graph);
    auto *bb3 = ir::BasicBlock::Create(&graph);
    auto *bb4 = ir::BasicBlock::Create(&graph);

    irBuilder.SetInsertionPoint(bb1);
    auto *v0 = irBuilder.CreateParam(ir::ResultType::S32, 0);
    auto *v1 = irBuilder.CreateConstInt(1);
    auto *v2 = irBuilder.CreateConstInt(2);
    [[maybe_unused]] auto *v3 = irBuilder.CreateBr(bb2);
}

TEST(PEEPHOLE_OPT, XorPeephole)
{
    auto graph = ir::Graph {};
    auto irBuilder = ir::IRBuilder {&graph};

    auto *bb1 = ir::BasicBlock::Create(&graph);
    auto *bb2 = ir::BasicBlock::Create(&graph);
    auto *bb3 = ir::BasicBlock::Create(&graph);
    auto *bb4 = ir::BasicBlock::Create(&graph);

    irBuilder.SetInsertionPoint(bb1);
    auto *v0 = irBuilder.CreateParam(ir::ResultType::S32, 0);
    auto *v1 = irBuilder.CreateConstInt(1);
    auto *v2 = irBuilder.CreateConstInt(2);
    [[maybe_unused]] auto *v3 = irBuilder.CreateBr(bb2);
}

/**
 *   Source Code:
 *       function foo(): int {
 *           let result = 1;
 *
 *           for (let i = 2; i <= value; i++)
 *               result = result * i;
 *           }
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.0:
 *           0.s32 Parameter 0                  // value
 *           1.s32 Constant 1
 *           2.s32 Constant 2
 *           3. Br BB.1
 *       BB.1:
 *           4p.s32 Phi v8:BB.2, v1:BB.0       // result
 *           5p.s32 Phi v9:BB.2, v2:BB.0       // i
 *           6.b Compare LE v5, v0
 *           7. If v6, BB.2, BB.3
 *       BB.2:
 *           8.s32 Mul v4, v5
 *           9.s32 Add v5, v1
 *           10. Br BB.1
 *       BB.3:
 *           11.s32 Return v4
 */
TEST(PEEPHOLE_OPT, ConstFolding)
{
    auto graph = ir::Graph {};
    auto irBuilder = ir::IRBuilder {&graph};

    auto *bb1 = ir::BasicBlock::Create(&graph);
    auto *bb2 = ir::BasicBlock::Create(&graph);
    auto *bb3 = ir::BasicBlock::Create(&graph);
    auto *bb4 = ir::BasicBlock::Create(&graph);

    irBuilder.SetInsertionPoint(bb1);
    auto *v0 = irBuilder.CreateParam(ir::ResultType::S32, 0);
    auto *v1 = irBuilder.CreateConstInt(1);
    auto *v2 = irBuilder.CreateConstInt(2);
    [[maybe_unused]] auto *v3 = irBuilder.CreateBr(bb2);
}

}  // namespace compiler::tests
