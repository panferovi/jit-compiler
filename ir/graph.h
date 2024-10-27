#ifndef IR_GRAPH_H
#define IR_GRAPH_H

#include "ir/id.h"
#include "ir/marker.h"
#include "utils/intrusive_list.h"
#include "utils/macros.h"

#include <cstdint>
#include <sstream>

namespace compiler::ir {

class BasicBlock;

class Graph {
public:
    explicit Graph() = default;
    NO_COPY_SEMANTIC(Graph);
    DEFAULT_MOVE_CTOR(Graph);
    NO_MOVE_OPERATOR(Graph);
    ~Graph();

    Id NewInstId()
    {
        return currentInstId_++;
    }

    Id NewBBId()
    {
        return currentBBId_++;
    }

    Marker NewMarker();

    void InsertBasicBlock(BasicBlock *bb);

    void Dump(std::stringstream &ss) const;

    BasicBlock *GetStartBlock();

    size_t GetBlocksCount() const;

private:
    Id currentBBId_ {0};
    Id currentInstId_ {0};
    uint64_t currentMarker_ {1};
    utils::IntrusiveList<BasicBlock> basicBlocks_;
};

}  // namespace compiler::ir

#endif  // IR_GRAPH_H
