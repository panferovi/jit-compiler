#include "ir/analysis.h"
#include "ir/basic_block.h"
#include "ir/graph.h"
#include "ir/instruction.h"
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
    auto dominatorsMap = BuildDominatorsMap(dfsSet);
    rootDominator_ = BuildDominatorTree(dominatorsMap);
}

DominatorsTree::DominatorsMap DominatorsTree::BuildDominatorsMap(const BBSet &dfsSet) const
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

DominatorsTree::BBDeque DominatorsTree::DominatorsMap::FindImmediateDominatees(BasicBlock *dominator) const
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

BasicBlock *DominatorsTree::BuildDominatorTree(const DominatorsTree::DominatorsMap &dominatorsMap) const
{
    auto *rootDominator = graph_->GetStartBlock();
    BuildTreeImpl(rootDominator, dominatorsMap);
    return rootDominator;
}

void DominatorsTree::BuildTreeImpl(BasicBlock *dominator, const DominatorsTree::DominatorsMap &dominatorsMap) const
{
    auto immDominatees = dominatorsMap.FindImmediateDominatees(dominator);
    for (auto *dominatee : immDominatees) {
        dominator->AddDominatee(dominatee);
        dominatee->SetDominator(dominator);
        BuildTreeImpl(dominatee, dominatorsMap);
    }
}

DominatorsTree::BBSet DominatorsTree::GetDominators(BasicBlock *bb) const
{
    ASSERT(rootDominator_ != nullptr);
    BBSet dominators;
    TraverseDominators(bb, [&dominators](BasicBlock *dominator) {
        dominators.insert(dominator);
        return false;
    });
    return dominators;
}

DominatorsTree::BBDeque DominatorsTree::GetOrderedDominators(BasicBlock *bb) const
{
    ASSERT(rootDominator_ != nullptr);
    BBDeque dominators;
    TraverseDominators(bb, [&dominators](BasicBlock *dominator) {
        dominators.push_back(dominator);
        return false;
    });
    return dominators;
}

BasicBlock *DominatorsTree::GetImmediateDominator(BasicBlock *bb) const
{
    ASSERT(rootDominator_ != nullptr);
    return bb->GetDominator();
}

BasicBlock *DominatorsTree::GetImmediateDominatorFor(BasicBlock *bb1, BasicBlock *bb2) const
{
    auto dominators1 = GetOrderedDominators(bb1);
    auto dominators2 = GetOrderedDominators(bb2);
    if (dominators1.empty() || dominators2.empty()) {
        return nullptr;
    }
    BasicBlock *commonDominator = nullptr;
    for (int idx1 = dominators1.size() - 1, idx2 = dominators2.size() - 1; idx1 >= 0 && idx2 >= 0; idx1--, idx2--) {
        if (dominators1[idx1] != dominators2[idx2]) {
            return commonDominator;
        }
        commonDominator = dominators1[idx1];
    }
    return commonDominator;
}

Instruction *DominatorsTree::GetImmediateDominatorFor(Instruction *inst1, Instruction *inst2) const
{
    auto *bb1 = inst1->GetBasicBlock();
    auto *bb2 = inst2->GetBasicBlock();
    if (bb1 != bb2) {
        return GetImmediateDominatorFor(bb1, bb2)->GetLastInstruction();
    }
    Instruction *commonDominator = nullptr;
    bb1->IterateOverInstructions([&commonDominator, inst1, inst2](Instruction *inst) {
        if (inst == inst1 || inst == inst2) {
            return true;
        }
        commonDominator = inst;
        return false;
    });
    return commonDominator;
}

bool DominatorsTree::DoesBlockDominatesOn(BasicBlock *dominatee, BasicBlock *dominator) const
{
    bool doesDominates = false;
    TraverseTree(dominator, [&doesDominates, dominatee, dominator](BasicBlock *bb) {
        if (dominatee == bb) {
            doesDominates = bb == dominator;
            return true;
        }
        return false;
    });
    return doesDominates;
}

bool DominatorsTree::DoesInstructionDominatesOn(Instruction *dominatee, Instruction *dominator) const
{
    auto *dominateeBlock = dominatee->GetBasicBlock();
    auto *dominatorBlock = dominator->GetBasicBlock();
    if (dominateeBlock != dominatorBlock) {
        return DoesBlockDominatesOn(dominateeBlock, dominatorBlock);
    }
    bool doesDominates = false;
    dominatorBlock->IterateOverInstructions([&doesDominates, dominatee, dominator](Instruction *inst) {
        if (inst == dominatee) {
            return true;
        }
        if (inst == dominator) {
            doesDominates = true;
            return true;
        }
        return false;
    });
    return doesDominates;
}

bool DominatorsTree::TraverseTree(BasicBlock *bb, const std::function<bool(BasicBlock *)> &callback) const
{
    ASSERT(bb != nullptr);
    if (callback(bb)) {
        return true;
    }
    for (auto &immDominatee : bb->GetImmediateDominatees()) {
        if (TraverseTree(immDominatee, callback)) {
            return true;
        }
    }
    return false;
}

void DominatorsTree::TraverseDominators(BasicBlock *bb, const std::function<bool(BasicBlock *)> &callback) const
{
    ASSERT(bb != nullptr);
    auto *dominator = bb->GetDominator();
    while (dominator != nullptr) {
        if (callback(dominator)) {
            return;
        }
        dominator = dominator->GetDominator();
    }
}

}  // namespace compiler::ir
