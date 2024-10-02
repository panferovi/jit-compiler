#ifndef IR_GRAPH_H
#define IR_GRAPH_H

#include "ir/id.h"
#include "utils/intrusive_list.h"

#include <cstdint>

namespace compiler::ir {

class BasicBlock;

class Graph {
public:
    Id GenerateInstId()
    {
        return currentInstId_++;
    }

    Id GenerateBBId()
    {
        return currentBBId_++;
    }

private:
    Id currentBBId_ {0};
    Id currentInstId_ {0};
    utils::IntrusiveList<BasicBlock> basicBlocks_;
};

}  // namespace compiler::ir

#endif  // IR_GRAPH_H
