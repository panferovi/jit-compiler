#ifndef IR_GRAPH_H
#define IR_GRAPH_H

#include "ir/id.h"
#include "utils/intrusive_list.h"
#include "utils/macros.h"

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

    Id GenerateInstId()
    {
        return currentInstId_++;
    }

    Id GenerateBBId()
    {
        return currentBBId_++;
    }

    void InsertBasicBlock(BasicBlock *bb);

    void Dump(std::stringstream &ss);

private:
    Id currentBBId_ {0};
    Id currentInstId_ {0};
    utils::IntrusiveList<BasicBlock> basicBlocks_;
};

}  // namespace compiler::ir

#endif  // IR_GRAPH_H
