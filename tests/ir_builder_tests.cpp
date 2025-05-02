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
TEST(IR_BUILDER, Factorial)
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
    [[maybe_unused]] auto *v11 = irBuilder.CreateRet(v4);

    v4->ResolveDependency(v1, bb1);
    v4->ResolveDependency(v8, bb3);

    v5->ResolveDependency(v2, bb1);
    v5->ResolveDependency(v9, bb3);

    std::stringstream ss;
    graph.Dump(ss);
    std::cout << ss.str() << std::endl;

    ASSERT(v0->GetOpcode() == ir::Opcode::PARAMETER);
    ASSERT(v0->GetInputs() == ir::Instruction::Inputs {});
    ASSERT(v0->GetUsers() == ir::Instruction::Users {v6});
    ASSERT(v0->GetBasicBlock() == bb1);

    ASSERT(v1->GetOpcode() == ir::Opcode::CONSTANT);
    ASSERT(v1->GetInputs() == ir::Instruction::Inputs {});
    ASSERT(v1->GetUsers() == ir::Instruction::Users({v4, v9}));
    ASSERT(v1->GetBasicBlock() == bb1);

    ASSERT(v2->GetOpcode() == ir::Opcode::CONSTANT);
    ASSERT(v2->GetInputs() == ir::Instruction::Inputs {});
    ASSERT(v2->GetUsers() == ir::Instruction::Users {v5});
    ASSERT(v2->GetBasicBlock() == bb1);

    ASSERT(v3->GetOpcode() == ir::Opcode::BRANCH);
    ASSERT(v3->GetInputs() == ir::Instruction::Inputs {});
    ASSERT(v3->GetUsers() == ir::Instruction::Users {});
    ASSERT(v3->GetBasicBlock() == bb1);

    ASSERT(bb1->GetTrueSuccessor() == bb2);
    ASSERT(bb1->GetFalseSuccessor() == nullptr);

    ASSERT(v4->GetOpcode() == ir::Opcode::PHI);
    ASSERT(v4->GetInputs() == ir::Instruction::Inputs {});
    ASSERT(v4->GetUsers() == ir::Instruction::Users({v8, v11}));
    ASSERT(v4->GetValueDependencies() == ir::PhiInst::ValueDependencies({{v1, {bb1}}, {v8, {bb3}}}));
    ASSERT(v4->GetBasicBlock() == bb2);

    ASSERT(v5->GetOpcode() == ir::Opcode::PHI);
    ASSERT(v5->GetInputs() == ir::Instruction::Inputs {});
    ASSERT(v5->GetUsers() == ir::Instruction::Users({v6, v8, v9}));
    ASSERT(v5->GetValueDependencies() == ir::PhiInst::ValueDependencies({{v2, {bb1}}, {v9, {bb3}}}));
    ASSERT(v5->GetBasicBlock() == bb2);

    ASSERT(v6->GetOpcode() == ir::Opcode::COMPARE);
    ASSERT(v6->GetFlags() == ir::CmpFlags::LE);
    ASSERT(v6->GetInputs() == ir::Instruction::Inputs({v5, v0}));
    ASSERT(v6->GetUsers() == ir::Instruction::Users {v7});
    ASSERT(v6->GetBasicBlock() == bb2);

    ASSERT(v7->GetOpcode() == ir::Opcode::COND_BRANCH);
    ASSERT(v7->GetInputs() == ir::Instruction::Inputs {v6});
    ASSERT(v7->GetUsers() == ir::Instruction::Users {});
    ASSERT(v7->GetBasicBlock() == bb2);

    ASSERT(bb2->GetTrueSuccessor() == bb3);
    ASSERT(bb2->GetFalseSuccessor() == bb4);

    ASSERT(v8->GetOpcode() == ir::Opcode::MUL);
    ASSERT(v8->GetInputs() == ir::Instruction::Inputs({v4, v5}));
    ASSERT(v8->GetUsers() == ir::Instruction::Users {v4});
    ASSERT(v8->GetBasicBlock() == bb3);

    ASSERT(v9->GetOpcode() == ir::Opcode::ADD);
    ASSERT(v9->GetInputs() == ir::Instruction::Inputs({v5, v1}));
    ASSERT(v9->GetUsers() == ir::Instruction::Users {v5});
    ASSERT(v9->GetBasicBlock() == bb3);

    ASSERT(v10->GetOpcode() == ir::Opcode::BRANCH);
    ASSERT(v10->GetInputs() == ir::Instruction::Inputs {});
    ASSERT(v10->GetUsers() == ir::Instruction::Users {});
    ASSERT(v10->GetBasicBlock() == bb3);

    ASSERT(bb3->GetTrueSuccessor() == bb2);
    ASSERT(bb3->GetFalseSuccessor() == nullptr);

    ASSERT(v11->GetOpcode() == ir::Opcode::RETURN);
    ASSERT(v11->GetInputs() == ir::Instruction::Inputs {v4});
    ASSERT(v11->GetUsers() == ir::Instruction::Users {});
    ASSERT(v11->GetBasicBlock() == bb4);

    ASSERT(bb4->GetTrueSuccessor() == nullptr);
    ASSERT(bb4->GetFalseSuccessor() == nullptr);
}

}  // namespace compiler::tests