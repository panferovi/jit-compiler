#include <gtest/gtest.h>
#include <sstream>
#include <iostream>

#include "ir/basic_block.h"
#include "ir/common.h"
#include "ir/graph.h"
#include "ir/ir_builder.h"
#include "ir/instruction.h"

namespace compiler::tests {

/**
 *   Source Code:
 *       function foo(value: int): int {
 *           let result = 1;
 *           for (let i = 2; i <= value; i++)
 *               result = result * i;
 *           }
 *           return result;
 *       }
 *
 *   IR Graph:
 *       BB.1:
 *           0.s32 Parameter 0                  // value
 *           1.s32 Constant 1
 *           2.s32 Constant 2
 *           3. Br BB.2
 *       BB.2:
 *           4p.s32 Phi v1:BB.1, v8:BB.3       // result
 *           5p.s32 Phi v2:BB.1, v9:BB.3       // i
 *           6.b Compare LE v5, v0
 *           7. If v6, BB.3, BB.4
 *       BB.3:
 *           8.s32 Mul v4, v5
 *           9.s32 Add v5, v1
 *           10. Br BB.2
 *       BB.4:
 *           11.s32 Return v4
 */
TEST(JIT_COMPILER, Factorial)
{
    auto graph = ir::Graph{};
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

    irBuilder.SetInsertionPoint(bb2);
    auto *v4 = irBuilder.CreatePhi(ir::ResultType::S32);
    auto *v5 = irBuilder.CreatePhi(ir::ResultType::S32);
    auto *v6 = irBuilder.CreateCmpLE(v5, v0);
    [[maybe_unused]] auto *v7 = irBuilder.CreateCondBr(v6, bb3, bb4);

    irBuilder.SetInsertionPoint(bb3);
    auto *v8 = irBuilder.CreateMul(v4, v5);
    auto *v9 = irBuilder.CreateAdd(v5, v1);
    [[maybe_unused]] auto *v10 = irBuilder.CreateBr(bb2);

    irBuilder.SetInsertionPoint(bb4);
    irBuilder.CreateRet(v4);

    v4->ResolveDependency(v1, bb1);
    v4->ResolveDependency(v8, bb3);

    v5->ResolveDependency(v2, bb1);
    v5->ResolveDependency(v9, bb3);

    std::stringstream ss;
    graph.Dump(ss);
    std::cout << ss.str() << std::endl;
}

}  // namespace compiler::tests