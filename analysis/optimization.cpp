#include "analysis/optimization.h"
#include "analysis/analysis.h"
#include "ir/basic_block.h"
#include "ir/common.h"
#include "ir/instruction.h"
#include "ir/graph.h"

#include <deque>
#include <unordered_map>

namespace compiler {

namespace {

ir::Instruction *CreateConstInst(ir::Graph *graph, ir::ResultType resType, int64_t constValue)
{
    auto *constBlock = graph->GetStartBlock();
    ir::Instruction *constInst = nullptr;
    constBlock->IterateOverInstructions([&constInst, constValue](ir::Instruction *inst) {
        if (inst->GetOpcode() == ir::Opcode::CONSTANT) {
            if (inst->As<ir::AssignInst>()->GetValue() == constValue) {
                constInst = inst;
                return true;
            }
        }
        return false;
    });
    if (constInst == nullptr) {
        constInst =
            new ir::AssignInst(constBlock, ir::InstId {graph->NewInstId()}, ir::Opcode::CONSTANT, resType, constValue);
        constBlock->InsertInstBack(constInst);
    }
    return constInst;
}

}  // namespace

/* static */
PeepHoleOptimizer::OptimizerMap PeepHoleOptimizer::CreateOptimizers()
{
    OptimizerMap optimizers {};
    optimizers.fill(OptimizeStub);
    optimizers[ir::OpcodeToIndex<ir::Opcode::ADD>()] = OptimizeAdd;
    return optimizers;
}

void PeepHoleOptimizer::Run()
{
    RPO rpo(graph_);
    rpo.Run();
    for (auto *bb : rpo.GetRpoVector()) {
        bb->IterateOverInstructions([](ir::Instruction *inst) {
            auto optimizerIdx = static_cast<uint32_t>(inst->GetOpcode());
            auto *optimizer = OpcodeToOptimizer[optimizerIdx];
            optimizer(inst);
            return false;
        });
    }
}

/* static */
PeepHoleOptimizer::OptStatus PeepHoleOptimizer::OptimizeConstArithm(ir::ArithmInst *inst, BothConstOpt bothConstOpt,
                                                                    FirstConstOpt firstConstOpt,
                                                                    SecondConstOpt secondConstOpt)
{
    auto [op1Const, op2Const] = inst->CheckInputsAreConst();
    if (!op1Const && !op2Const) {
        return OptStatus::NO_OPT;
    }
    ir::Instruction *newInst = nullptr;
    auto *op1 = inst->GetFirstOp();
    auto *op2 = inst->GetSecondOp();
    if (op1Const && op2Const) {
        auto constValue = bothConstOpt(op1->As<ir::AssignInst>()->GetValue(), op2->As<ir::AssignInst>()->GetValue());
        newInst = CreateConstInst(inst->GetBasicBlock()->GetGraph(), ir::CombineResultType(op1, op2), constValue);
    } else {
        newInst = op1Const ? firstConstOpt(op1->As<ir::AssignInst>()->GetValue(), inst)
                           : secondConstOpt(inst, op2->As<ir::AssignInst>()->GetValue());
    }
    if (newInst != nullptr) {
        ir::Instruction::UpdateUsersAndEleminate(inst, newInst);
        return OptStatus::OPT;
    }
    return OptStatus::CANT_OPT;
}

/* static */
PeepHoleOptimizer::OptStatus PeepHoleOptimizer::OptimizeSameInputs(ir::ArithmInst *inst, SameInputsOpt sameInputsOpt)
{
    if (inst->GetFirstOp() != inst->GetSecondOp()) {
        return OptStatus::NO_OPT;
    }
    auto *newInst = sameInputsOpt(inst);
    if (newInst != nullptr) {
        ir::Instruction::UpdateUsersAndEleminate(inst, newInst);
        return OptStatus::OPT;
    }
    return OptStatus::CANT_OPT;
}

/* static */
void PeepHoleOptimizer::OptimizeAdd(ir::Instruction *addInst)
{
    auto bothConst = [](int64_t op1, int64_t op2) { return op1 + op2; };
    auto firstConst = [](int64_t op1, ir::Instruction *inst) {
        if (op1 == 0) {
            return inst->GetSecondOp();
        }
        return ir::Instruction::EmptyInst;
    };
    auto secondConst = [](ir::Instruction *inst, int64_t op2) {
        if (op2 == 0) {
            return inst->GetFirstOp();
        }
        return ir::Instruction::EmptyInst;
    };
    auto sameInputsOpt = [](ir::Instruction *inst) {
        auto bb = inst->GetBasicBlock();
        auto *graph = bb->GetGraph();
        auto *constOne = CreateConstInst(graph, ir::ResultType::U8, 1U);
        auto newInst = new ir::ArithmInst(bb, inst->GetInstId(), ir::Opcode::SHL, inst->GetResultType(),
                                          {inst->GetFirstOp(), constOne});
        newInst->InsertInstBefore(inst);
        return newInst->As<ir::Instruction>();
    };
    auto status = OptimizeConstArithm(addInst->As<ir::ArithmInst>(), bothConst, firstConst, secondConst);
    if (status == OptStatus::NO_OPT) {
        status = OptimizeSameInputs(addInst->As<ir::ArithmInst>(), sameInputsOpt);
    }
}

/* static */
void PeepHoleOptimizer::OptimizeShl(ir::Instruction *shlInst)
{
    auto bothConst = [](int64_t op1, int64_t op2) { return op1 << op2; };
    auto firstConst = [](int64_t op1, ir::Instruction *inst) {
        if (op1 == 0) {
            return CreateConstInst(inst->GetBasicBlock()->GetGraph(), ir::ResultType::U8, 0);
        }
        return ir::Instruction::EmptyInst;
    };
    auto secondConst = [](ir::Instruction *inst, int64_t op2) {
        if (op2 == 0) {
            return inst->GetFirstOp();
        }
        return ir::Instruction::EmptyInst;
    };
    [[maybe_unused]] auto _ = OptimizeConstArithm(shlInst->As<ir::ArithmInst>(), bothConst, firstConst, secondConst);
}

/* static */
void PeepHoleOptimizer::OptimizeXor(ir::Instruction *xorInst)
{
    auto bothConst = [](int64_t op1, int64_t op2) { return op1 ^ op2; };
    auto firstConst = [](int64_t op1, ir::Instruction *inst) {
        if (op1 == 0) {
            return inst->GetSecondOp();
        }
        return ir::Instruction::EmptyInst;
    };
    auto secondConst = [](ir::Instruction *inst, int64_t op2) {
        if (op2 == 0) {
            return inst->GetFirstOp();
        }
        return ir::Instruction::EmptyInst;
    };
    auto sameInputsOpt = [](ir::Instruction *inst) {
        return CreateConstInst(inst->GetBasicBlock()->GetGraph(), ir::ResultType::U8, 0);
    };
    auto status = OptimizeConstArithm(xorInst->As<ir::ArithmInst>(), bothConst, firstConst, secondConst);
    if (status == OptStatus::NO_OPT) {
        status = OptimizeSameInputs(xorInst->As<ir::ArithmInst>(), sameInputsOpt);
    }
}

void CheckOptimizer::Run()
{
    DominatorsTree domTree(graph_);
    domTree.Run();

    auto eleminateChecks = [&domTree](std::deque<ir::Instruction *> &cheks) {
        while (!cheks.empty()) {
            auto *check = cheks.front();
            cheks.pop_front();
            for (auto otherCheckIt = cheks.begin(); otherCheckIt != cheks.end();) {
                if (domTree.DoesInstructionDominatesOn(*otherCheckIt, check)) {
                    Instruction::Eleminate(*otherCheckIt);
                    otherCheckIt = cheks.erase(otherCheckIt);
                } else if (domTree.DoesInstructionDominatesOn(check, *otherCheckIt)) {
                    Instruction::Eleminate(check);
                    break;
                } else {
                    ++otherCheckIt;
                }
            }
        }
    };

    RPO rpo(graph_);
    rpo.Run();

    for (auto *bb : rpo.GetRpoVector()) {
        bb->IterateOverInstructions([&eleminateChecks](ir::Instruction *inst) {
            if (inst->GetOpcode() != ir::Opcode::MEM) {
                return false;
            }
            std::unordered_map<ir::CheckType, std::deque<ir::Instruction *>> checks;
            for (auto *user : inst->GetUsers()) {
                if (user->GetOpcode() == ir::Opcode::CHECK) {
                    checks[user->As<ir::CheckInst>()->GetType()].push_back(user);
                }
            }
            for (auto &[_, sameTChecks] : checks) {
                eleminateChecks(sameTChecks);
            }
            return false;
        });
    }
}

}  // namespace compiler
