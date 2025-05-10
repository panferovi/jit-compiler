#ifndef IR_CALL_GRAPH_H
#define IR_CALL_GRAPH_H

#include "ir/common.h"
#include "utils/macros.h"

#include <unordered_map>

namespace compiler::ir {

class Graph;

class CallGraph {
public:
    explicit CallGraph() = default;
    NO_COPY_SEMANTIC(CallGraph);
    DEFAULT_MOVE_CTOR(CallGraph);
    NO_MOVE_OPERATOR(CallGraph);
    ~CallGraph();

    MethodId LinkGraph(std::string_view methodName, Graph *graph);

    Graph *GetGraphByMethodId(MethodId methodId) const;

private:
    MethodId currentMethodId_;
    std::unordered_map<std::string, MethodId> methodNameToId_;
    std::unordered_map<MethodId, Graph *> methodIdToGraph_;
};

}  // namespace compiler::ir

#endif  // IR_CALL_GRAPH_H