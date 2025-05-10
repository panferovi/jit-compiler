#include "ir/call_graph.h"
#include "ir/common.h"

namespace compiler::ir {

MethodId CallGraph::LinkGraph(std::string_view methodName, Graph *graph)
{
    [[maybe_unused]] auto inserted = methodNameToId_.insert({std::string(methodName), currentMethodId_});
    ASSERT(inserted.second);
    methodIdToGraph_.insert({currentMethodId_, graph});
    return currentMethodId_++;
}

Graph *CallGraph::GetGraphByMethodId(MethodId methodId) const
{
    auto graphIt = methodIdToGraph_.find(methodId);
    ASSERT(graphIt != methodIdToGraph_.end());
    return graphIt->second;
}

}  // namespace compiler::ir
