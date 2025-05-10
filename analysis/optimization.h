#ifndef ANALYSIS_OPTIMIZATION_H
#define ANALYSIS_OPTIMIZATION_H

#include "ir/common.h"

#include <array>
#include <unordered_map>

namespace compiler {

namespace ir {
class Graph;
class BasicBlock;
class Instruction;
class ArithmInst;
class CallStaticInst;
}  // namespace ir

class PeepHoleOptimizer {
public:
    explicit PeepHoleOptimizer(ir::Graph *graph) : graph_(graph) {}

    void Run();

private:
    enum class OptStatus { NO_OPT, OPT, CANT_OPT };

    static void OptimizeStub([[maybe_unused]] ir::Instruction *inst) {}
    static void OptimizeAdd(ir::Instruction *addInst);
    static void OptimizeShl(ir::Instruction *shlInst);
    static void OptimizeXor(ir::Instruction *xorInst);
    static void OptimizePhi(ir::Instruction *phiInst);

    using BothConstOpt = int64_t (*)(int64_t op1, int64_t op2);
    using FirstConstOpt = ir::Instruction *(*)(int64_t op1, ir::Instruction *inst);
    using SecondConstOpt = ir::Instruction *(*)(ir::Instruction *inst, int64_t op2);
    static OptStatus OptimizeConstArithm(ir::ArithmInst *inst, BothConstOpt bothConstOpt, FirstConstOpt firstConstOpt,
                                         SecondConstOpt secondConstOpt);

    using SameInputsOpt = ir::Instruction *(*)(ir::Instruction *inst);
    static OptStatus OptimizeSameInputs(ir::ArithmInst *inst, SameInputsOpt sameInputsOpt);

    static constexpr auto OptimizerCnt = static_cast<uint32_t>(ir::Opcode::COUNT);
    using Optimizer = void (*)(ir::Instruction *inst);
    using OptimizerMap = std::array<Optimizer, OptimizerCnt>;

    static OptimizerMap CreateOptimizers();
    static inline OptimizerMap OpcodeToOptimizer = CreateOptimizers();

    ir::Graph *graph_;
};

class CheckOptimizer {
public:
    explicit CheckOptimizer(ir::Graph *graph) : graph_(graph) {}

    void Run();

private:
    static bool OptimizePredStub([[maybe_unused]] ir::Instruction *inst1, [[maybe_unused]] ir::Instruction *inst2)
    {
        return false;
    }

    static bool OptimizePredNil(ir::Instruction *nilInst1, ir::Instruction *nilInst2);
    static bool OptimizePredBounds(ir::Instruction *boundsInst1, ir::Instruction *boundsInst2);

    static constexpr auto OptimizerCnt = static_cast<uint32_t>(ir::CheckType::COUNT);
    using OptimizerPredicate = bool (*)(ir::Instruction *inst1, ir::Instruction *inst2);
    using PredicatesMap = std::array<OptimizerPredicate, OptimizerCnt>;

    static PredicatesMap CreateOptimizerPredicates();
    static inline PredicatesMap TypeToOptimizerPredicate = CreateOptimizerPredicates();

    ir::Graph *graph_;
};

class InliningOptimizer {
public:
    explicit InliningOptimizer(ir::Graph *graph) : graph_(graph) {}

    void Run();

private:
    template <typename T>
    using Mapping = std::unordered_map<T *, T *>;

    /// @return first and last blocks of inlined graph
    static std::pair<ir::BasicBlock *, ir::BasicBlock *> CloneCalleeGraph(ir::CallStaticInst *callInst,
                                                                          ir::Graph *calleeGraph);

    /// @return last block of inlined graph
    static ir::BasicBlock *UpdateDataFlowOfInlinedGraph(ir::CallStaticInst *callInst,
                                                        Mapping<ir::BasicBlock> oldToNewBB,
                                                        Mapping<ir::Instruction> oldToNewInst);

    static void MergeDataFLow(ir::CallStaticInst *callInst, ir::BasicBlock *firstCalleeBB, ir::BasicBlock *postCallBB);

    ir::Graph *graph_;
};

}  // namespace compiler

#endif  // ANALYSIS_OPTIMIZATION_H
