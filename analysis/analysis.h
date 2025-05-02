#ifndef ANALYSIS_ANALYSIS_H
#define ANALYSIS_ANALYSIS_H

#include "ir/marker.h"

#include <set>
#include <deque>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <functional>

namespace compiler {

namespace ir {
class Graph;
class BasicBlock;
class Instruction;
}  // namespace ir

using ir::BasicBlock;
using ir::Graph;
using ir::Instruction;
using ir::Marker;

class DFS {
public:
    using DfsVector = std::vector<BasicBlock *>;

    explicit DFS(Graph *graph) : graph_(graph) {}

    const DfsVector &GetDfsVector() const
    {
        return dfsVector_;
    }

    std::set<BasicBlock *> CreateDfsBBSet() const
    {
        return {dfsVector_.begin(), dfsVector_.end()};
    }

    void SetMarker(Marker marker)
    {
        marker_ = marker;
    }

    Marker GetMarker()
    {
        return marker_;
    }

    void Run();

private:
    void DFSImpl(BasicBlock *bb);

    Graph *graph_;
    Marker marker_;
    DfsVector dfsVector_;
};

class RPO {
public:
    using RpoVector = std::vector<BasicBlock *>;

    explicit RPO(Graph *graph) : graph_(graph) {}

    const RpoVector &GetRpoVector() const
    {
        return rpoVector_;
    }

    void Run();

private:
    void RPOImpl(BasicBlock *bb, size_t *blocksCount);

    Graph *graph_;
    Marker marker_;
    RpoVector rpoVector_;
};

class DominatorsTree {
public:
    using BBSet = std::set<BasicBlock *>;
    using BBDeque = std::deque<BasicBlock *>;

    explicit DominatorsTree(Graph *graph) : graph_(graph) {}

    void Run();

    BBSet GetDominators(BasicBlock *bb) const;

    BBDeque GetOrderedDominators(BasicBlock *bb) const;

    BasicBlock *GetImmediateDominator(BasicBlock *bb) const;

    BasicBlock *GetImmediateDominatorFor(BasicBlock *bb1, BasicBlock *bb2) const;

    Instruction *GetImmediateDominatorFor(Instruction *inst1, Instruction *inst2) const;

    bool DoesBlockDominatesOn(BasicBlock *dominatee, BasicBlock *dominator) const;

    bool DoesInstructionDominatesOn(Instruction *dominatee, Instruction *dominator) const;

private:
    class DominatorsMap {
    public:
        struct DominatorsComp {
            bool operator()(BasicBlock *const &bb1, BasicBlock *const &bb2) const;
        };
        using Dominators = std::unordered_map<BasicBlock *, std::set<BasicBlock *, DominatorsComp>>;

        explicit DominatorsMap() = default;
        explicit DominatorsMap(Dominators dominatorsMap) : dominatorsMap_(std::move(dominatorsMap)) {}

        BBDeque FindImmediateDominatees(BasicBlock *dominator) const;

    private:
        Dominators dominatorsMap_;
    };

    DominatorsMap BuildDominatorsMap(const BBSet &dfsSet) const;

    bool TraverseTree(BasicBlock *bb, const std::function<bool(BasicBlock *)> &callback) const;

    void TraverseDominators(BasicBlock *bb, const std::function<bool(BasicBlock *)> &callback) const;

    BasicBlock *BuildDominatorTree(const DominatorsMap &dominatorsMap) const;

    void BuildTreeImpl(BasicBlock *dominator, const DominatorsMap &dominatorsMap) const;

    Graph *graph_;
    Marker marker_;
    BasicBlock *rootDominator_ {nullptr};
};

}  // namespace compiler

#endif  // ANALYSIS_ANALYSIS_H
