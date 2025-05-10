#include "ir/graph.h"
#include "ir/basic_block.h"
#include "ir/call_graph.h"
#include "ir/common.h"

namespace compiler::ir {

void Graph::InsertBasicBlock(BasicBlock *bb)
{
    basicBlocks_.PushBack(bb);
}

void Graph::Dump(std::stringstream &ss) const
{
    for (auto *bb : basicBlocks_) {
        bb->Dump(ss);
    }
}

BasicBlock *Graph::GetStartBlock()
{
    if (basicBlocks_.IsEmpty()) {
        return nullptr;
    }
    return *basicBlocks_.begin();
}

size_t Graph::GetBlocksCount() const
{
    return basicBlocks_.Size();
}

Marker Graph::NewMarker()
{
    // markers are expired
    ASSERT(currentMarker_ != 0);
    auto marker = Marker(currentMarker_);
    currentMarker_ <<= 1ULL;
    return marker;
}

Graph::~Graph()
{
    while (basicBlocks_.NonEmpty()) {
        auto *bb = basicBlocks_.PopFront();
        delete bb;
    }
}

void Graph::LinkToCallGraph(std::string_view methodName)
{
    id_ = callGraph_->LinkGraph(methodName, this);
}

Graph *Graph::GetGraphByMethodId(MethodId methodId) const
{
    ASSERT(callGraph_ != nullptr);
    return callGraph_->GetGraphByMethodId(methodId);
}

void Graph::IterateOverBlocks(const BlockVisitor &visitor)
{
    for (auto blockIt = basicBlocks_.begin(); blockIt != basicBlocks_.end();) {
        // visitor can link new blocks, but not eliminate block
        visitor(*blockIt);
        blockIt = std::next(blockIt);
        ASSERT(blockIt != nullptr);
    }
}

}  // namespace compiler::ir