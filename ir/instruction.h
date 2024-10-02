#ifndef IR_INSTRUCTION_H
#define IR_INSTRUCTION_H

#include "ir/id.h"
#include "ir/common.h"
#include "utils/macros.h"
#include "utils/intrusive_list.h"

#include <list>
#include <unordered_map>
#include <initializer_list>

namespace compiler::ir {

class BasicBlock;
class Instruction;

using InstProxyList = std::initializer_list<Instruction *>;

class Instruction : public utils::IntrusiveListNode<Instruction> {
public:
    using Inputs = std::list<Instruction *>;
    using Users = std::list<Instruction *>;
    using ListNode = utils::IntrusiveListNode<Instruction>;

    Instruction(InstId id, Opcode op, ResultType resType, InstProxyList inputs = {})
        : ListNode(), instId_(id), op_(op), resType_(resType), inputs_(inputs)
    {
        ASSERT(op != Opcode::INVALID);
        ASSERT(resType != ResultType::INVALID);
    }

    virtual ~Instruction() = default;

    InstId GetId() const
    {
        return instId_;
    }

    Opcode GetOpcode() const
    {
        return op_;
    }

    ResultType GetResultType() const
    {
        return resType_;
    }

    const Inputs &GetInputs() const
    {
        return inputs_;
    }

    const Inputs &GetUsers() const
    {
        return inputs_;
    }

    void AddUsers(Instruction *user)
    {
        users_.push_back(user);
    }

    void AddUsers(InstProxyList users)
    {
        users_.insert(std::prev(users_.end()), users);
    }

private:
    InstId instId_;
    Opcode op_;
    ResultType resType_;
    Inputs inputs_;
    Users users_;
};

class AssignInst : public Instruction {
public:
    using ConstOrParamId = int64_t;

    AssignInst(InstId id, Opcode op, ResultType resType, ConstOrParamId value)
        : Instruction(id, op, resType), value_(value)
    {
        ASSERT(resType != ResultType::VOID);
    }

    ConstOrParamId GetValue()
    {
        return value_;
    }

private:
    ConstOrParamId value_;
};

class ArithmInst : public Instruction {
public:
    ArithmInst(InstId id, Opcode op, ResultType resType, InstProxyList inputs) : Instruction(id, op, resType, inputs)
    {
        ASSERT(resType != ResultType::VOID);
        ASSERT(inputs.size() == 2);
    }
};

class LogicInst : public Instruction {
public:
    LogicInst(InstId id, Opcode op, InstProxyList inputs, CmpFlags flags)
        : Instruction(id, op, ResultType::BOOL, inputs), flags_(flags)
    {
        ASSERT(flags != CmpFlags::INVALID);
        ASSERT(inputs.size() == 2);
    }

    CmpFlags GetFlags() const
    {
        return flags_;
    }

private:
    CmpFlags flags_;
};

class BranchInst : public Instruction {
public:
    BranchInst(InstId id, Opcode op, InstProxyList inputs) : Instruction(id, op, ResultType::VOID, inputs)
    {
        ASSERT((op == Opcode::BRANCH && inputs.size() == 0) || inputs.size() == 1);
    }
};

class ReturnInst : public Instruction {
public:
    ReturnInst(InstId id, Opcode op, ResultType resType, InstProxyList inputs) : Instruction(id, op, resType, inputs)
    {
        ASSERT((resType == ResultType::VOID && inputs.size() == 0) || inputs.size() == 1);
    }
};

class PhiInst : public Instruction {
public:
    PhiInst(InstId id, ResultType resType) : Instruction(id, Opcode::PHI, resType)
    {
        ASSERT(resType != ResultType::VOID);
    }

    void ResolveDependency(Instruction *value, BasicBlock *bb)
    {
        ASSERT(value->GetResultType() == GetResultType());
        valueDeps_.insert(std::pair {value, bb});
    }

private:
    using ValueDependencies = std::unordered_multimap<Instruction *, BasicBlock *>;

    ValueDependencies valueDeps_;
};

}  // namespace compiler::ir

#endif  // IR_INSTRUCTION_H
