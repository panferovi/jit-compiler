#ifndef IR_GRAPH_H
#define IR_GRAPH_H

#include "ir/id.h"
#include "ir/marker.h"
#include "ir/common.h"
#include "utils/intrusive_list.h"
#include "utils/macros.h"

#include <cstdint>
#include <sstream>
#include <functional>

namespace compiler::ir {

class BasicBlock;
class CallGraph;

class Graph {
public:
    using BlockVisitor = std::function<void(BasicBlock *)>;

    explicit Graph() = default;
    NO_COPY_SEMANTIC(Graph);
    DEFAULT_MOVE_CTOR(Graph);
    NO_MOVE_OPERATOR(Graph);
    ~Graph();

    explicit Graph(CallGraph *callGraph, std::string_view methodName) : callGraph_(callGraph)
    {
        LinkToCallGraph(methodName);
    }

    Id NewInstId()
    {
        return currentInstId_++;
    }

    Id NewBBId()
    {
        return currentBBId_++;
    }

    MethodId GetMethodId() const
    {
        return id_;
    }

    Marker NewMarker();

    void InsertBasicBlock(BasicBlock *bb);

    void Dump(std::stringstream &ss) const;

    BasicBlock *GetStartBlock();

    size_t GetBlocksCount() const;

    Graph *GetGraphByMethodId(MethodId methodId) const;

    void IterateOverBlocks(const BlockVisitor &visitor);

private:
    void LinkToCallGraph(std::string_view methodName);

    CallGraph *callGraph_;
    MethodId id_ {0};
    Id currentBBId_ {0};
    Id currentInstId_ {0};
    uint64_t currentMarker_ {1};
    utils::IntrusiveList<BasicBlock> basicBlocks_;
};

}  // namespace compiler::ir

#endif  // IR_GRAPH_H
