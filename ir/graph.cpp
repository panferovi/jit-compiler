#include "ir/graph.h"
#include "ir/basic_block.h"

namespace compiler::ir {

void Graph::InsertBasicBlock(BasicBlock *bb)
{
    basicBlocks_.PushBack(bb);
}

void Graph::Dump(std::stringstream &ss)
{
    for (auto *bb : basicBlocks_) {
        bb->Dump(ss);
    }
}

Graph::~Graph()
{
    while (basicBlocks_.NonEmpty()) {
        auto *bb = basicBlocks_.PopFront();
        delete bb;
    }
}

}  // namespace compiler::ir