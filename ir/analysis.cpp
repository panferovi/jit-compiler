#include "ir/analysis.h"
#include "ir/basic_block.h"
#include "ir/graph.h"
#include "ir/marker.h"
#include "utils/macros.h"

#include <algorithm>

namespace compiler::ir {

void DFS::Run()
{
    dfsVector_.clear();
    marker_ = marker_.IsEmpty() ? graph_->NewMarker() : marker_;
    DFSImpl(graph_->GetStartBlock());
    for (auto *bb : dfsVector_) {
        bb->Unmark(marker_);
    }
}

void DFS::DFSImpl(BasicBlock *bb)
{
    ASSERT(bb != nullptr);
    bb->Mark(marker_);
    dfsVector_.push_back(bb);
    for (auto *succBlock : bb->GetSuccessors()) {
        if (!succBlock->IsMarked(marker_)) {
            DFSImpl(succBlock);
        }
    }
}

void RPO::Run()
{
    auto blocksCount = graph_->GetBlocksCount();
    auto *bb = graph_->GetStartBlock();
    rpoVector_.resize(blocksCount);
    marker_ = marker_.IsEmpty() ? graph_->NewMarker() : marker_;
    RPOImpl(bb, &blocksCount);
    for (auto *bb : rpoVector_) {
        bb->Unmark(marker_);
    }
}

void RPO::RPOImpl(BasicBlock *bb, size_t *blocksCount)
{
    ASSERT(bb != nullptr);
    bb->Mark(marker_);

    for (auto *succBlock : bb->GetSuccessors()) {
        if (!succBlock->IsMarked(marker_)) {
            RPOImpl(succBlock, blocksCount);
        }
    }

    ASSERT(blocksCount != nullptr && *blocksCount > 0);
    rpoVector_[--(*blocksCount)] = bb;
}

void DominatorsTree::Run()
{
    auto dfs = DFS {graph_};
    marker_ = marker_.IsEmpty() ? graph_->NewMarker() : marker_;
    dfs.SetMarker(marker_);
    dfs.Run();

    auto &dfsVec = dfs.GetDfsVector();
    for (auto idx = 0U; idx < dfsVec.size(); ++idx) {
        dfsVec[idx]->SetDfsOrder(idx);
    }

    auto dfsSet = dfs.CreateDfsBBSet();
    ASSERT(dfsSet.size() == dfsVec.size());
    dominatorsMap_ = BuildDominatorsMap(dfsSet);
    rootDominator_ = BuildDominatorTree();
}

DominatorsTree::DominatorsMap DominatorsTree::BuildDominatorsMap(const BBSet &dfsSet)
{
    ASSERT(!marker_.IsEmpty());

    auto dfs = DFS {graph_};
    dfs.SetMarker(marker_);

    DominatorsMap::Dominators dominatorsMap;
    auto *startBB = graph_->GetStartBlock();

    for (auto *dominator : dfsSet) {
        // skip source
        if (dominator == startBB) {
            continue;
        }
        // startBB dominates each block
        dominatorsMap[dominator].insert(startBB);
        // skip dominator in dfs
        dominator->Mark(marker_);
        dfs.Run();
        dominator->Unmark(marker_);

        auto dfsSubset = dfs.CreateDfsBBSet();
        auto dominatedSet = BBSet {};
        std::set_difference(dfsSet.begin(), dfsSet.end(), dfsSubset.begin(), dfsSubset.end(),
                            std::inserter(dominatedSet, dominatedSet.begin()));
        dominatedSet.erase(dominator);

        for (auto *dominatee : dominatedSet) {
            dominatorsMap[dominatee].insert(dominator);
        }
    }
    return DominatorsMap(std::move(dominatorsMap));
}

bool DominatorsTree::DominatorsMap::DominatorsComp::operator()(BasicBlock *const &bb1, BasicBlock *const &bb2) const
{
    return bb1->GetDfsOrder() < bb2->GetDfsOrder();
}

DominatorsTree::BBDeque DominatorsTree::DominatorsMap::FindImmediateDominatedBlocks(BasicBlock *dominator)
{
    BBDeque immDominatees;
    for (auto &[dominatee, dominators] : dominatorsMap_) {
        auto *immediateDominator = *dominators.rbegin();
        if (immediateDominator == dominator) {
            immDominatees.push_back(dominatee);
        }
    }
    return immDominatees;
}

DominatorsTree::Node *DominatorsTree::BuildDominatorTree()
{
    auto *rootDominator = CreateNode(graph_->GetStartBlock(), nullptr);
    BuildTreeImpl(rootDominator);
    return rootDominator;
}

void DominatorsTree::BuildTreeImpl(Node *dominator)
{
    auto immDominatees = dominatorsMap_.FindImmediateDominatedBlocks(dominator->GetBasicBlock());
    for (auto *dominatee : immDominatees) {
        auto *node = CreateNode(dominatee, dominator);
        dominator->AddDominatee(node);
        BuildTreeImpl(node);
    }
}

DominatorsTree::Node *DominatorsTree::CreateNode(BasicBlock *block, Node *dominator)
{
    ASSERT(block != nullptr);
    return new Node {block, dominator};
}

void DominatorsTree::Node::AddDominatee(Node *dominatee)
{
    immDominatees_.push_back(dominatee);
}

BasicBlock *DominatorsTree::Node::GetBasicBlock() const
{
    return block_;
}

DominatorsTree::Node *DominatorsTree::Node::GetDominator() const
{
    return dominator_;
}

const std::deque<DominatorsTree::Node *> &DominatorsTree::Node::GetImmediateDominatees() const
{
    return immDominatees_;
}

DominatorsTree::BBSet DominatorsTree::GetDominators(BasicBlock *bb)
{
    BBSet dominators;
    TraverseTree(rootDominator_, [&dominators, bb](Node *node) {
        if (node->GetBasicBlock() != bb) {
            return false;
        }
        // dominator is nullptr in case of root
        for (auto *dominator = node->GetDominator(); dominator != nullptr; dominator = dominator->GetDominator()) {
            dominators.insert(dominator != nullptr ? dominator->GetBasicBlock() : nullptr);
        }
        return true;
    });
    return dominators;
}

BasicBlock *DominatorsTree::GetImmediateDominator(BasicBlock *bb)
{
    BasicBlock *immDominator = nullptr;
    TraverseTree(rootDominator_, [&immDominator, bb](Node *node) {
        if (node->GetBasicBlock() != bb) {
            return false;
        }
        // dominator is nullptr in case of root
        auto *dominator = node->GetDominator();
        immDominator = dominator != nullptr ? dominator->GetBasicBlock() : nullptr;
        return true;
    });
    return immDominator;
}

bool DominatorsTree::TraverseTree(Node *node, const std::function<bool(Node *)> &callback)
{
    ASSERT(node != nullptr);
    if (callback(node)) {
        return true;
    }
    for (auto &immDominatee : node->GetImmediateDominatees()) {
        if (TraverseTree(immDominatee, callback)) {
            return true;
        }
    }
    return false;
}

}  // namespace compiler::ir
