#ifndef IR_ANALYSIS_H
#define IR_ANALYSIS_H

#include "ir/marker.h"

#include <set>
#include <deque>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <functional>

namespace compiler::ir {

class Graph;
class BasicBlock;

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

    BBSet GetDominators(BasicBlock *bb);

    BasicBlock *GetImmediateDominator(BasicBlock *bb);

private:
    class DominatorsMap {
    public:
        struct DominatorsComp {
            bool operator()(BasicBlock *const &bb1, BasicBlock *const &bb2) const;
        };
        using Dominators = std::unordered_map<BasicBlock *, std::set<BasicBlock *, DominatorsComp>>;

        explicit DominatorsMap() = default;
        explicit DominatorsMap(Dominators dominatorsMap) : dominatorsMap_(std::move(dominatorsMap)) {}

        BBDeque FindImmediateDominatedBlocks(BasicBlock *dominator);

    private:
        Dominators dominatorsMap_;
    };

    DominatorsMap BuildDominatorsMap(const BBSet &dfsSet);

    class Node {
    public:
        explicit Node(BasicBlock *block, Node *dominator) : block_(block), dominator_(dominator) {}

        void AddDominatee(Node *dominatee);

        BasicBlock *GetBasicBlock() const;

        Node *GetDominator() const;

        const std::deque<Node *> &GetImmediateDominatees() const;

    private:
        BasicBlock *block_ {nullptr};
        Node *dominator_ {nullptr};
        std::deque<Node *> immDominatees_;
    };

    bool TraverseTree(Node *node, const std::function<bool(Node *)> &callback);

    Node *BuildDominatorTree();

    void BuildTreeImpl(Node *dominator);

    Node *CreateNode(BasicBlock *block, Node *dominator);

    Graph *graph_;
    Marker marker_;
    DominatorsMap dominatorsMap_;
    Node *rootDominator_ {nullptr};
};

}  // namespace compiler::ir

#endif  // IR_ANALYSIS_H
