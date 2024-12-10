#ifndef IR_INSTRUCTION_H
#define IR_INSTRUCTION_H

#include "ir/id.h"
#include "ir/common.h"
#include "utils/macros.h"
#include "utils/intrusive_list.h"

#include <list>
#include <sstream>
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

    Instruction(BasicBlock *ownBB, InstId id, Opcode op, ResultType resType, InstProxyList inputs = {})
        : ListNode(), ownBB_(ownBB), instId_(id), op_(op), resType_(resType), inputs_(inputs)
    {
        ASSERT(op != Opcode::INVALID);
        ASSERT(resType != ResultType::INVALID);
    }

    BasicBlock *GetBasicBlock() const
    {
        return ownBB_;
    }

    InstId GetInstId() const
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

    const Users &GetUsers() const
    {
        return users_;
    }

    void SetBasicBlock(BasicBlock *bb)
    {
        ownerBB_ = bb;
    }

    void AddUsers(Instruction *user)
    {
        users_.push_back(user);
    }

    void AddUsers(InstProxyList users)
    {
        users_.insert(std::prev(users_.end()), users);
    }

    void InsertInstBefore(Instruction *insertionPoint)
    {
        LinkBefore(insertionPoint);
    }

    virtual ~Instruction() = default;

    virtual void Dump(std::stringstream &ss) const;

private:
    BasicBlock *ownBB_;
    InstId instId_;
    Opcode op_;
    ResultType resType_;
    Inputs inputs_;
    Users users_;
    BasicBlock *ownerBB_ = nullptr;
};

class AssignInst : public Instruction {
public:
    using ConstOrParamId = int64_t;

    AssignInst(BasicBlock *ownBB, InstId id, Opcode op, ResultType resType, ConstOrParamId value)
        : Instruction(ownBB, id, op, resType), value_(value)
    {
        ASSERT(resType != ResultType::VOID);
    }

    ConstOrParamId GetValue() const
    {
        return value_;
    }

    void Dump(std::stringstream &ss) const override;

private:
    ConstOrParamId value_;
};

class ArithmInst : public Instruction {
public:
    ArithmInst(BasicBlock *ownBB, InstId id, Opcode op, ResultType resType, InstProxyList inputs)
        : Instruction(ownBB, id, op, resType, inputs)
    {
        ASSERT(resType != ResultType::VOID);
        ASSERT(inputs.size() == 2);
    }

    void Dump(std::stringstream &ss) const override;
};

class LogicInst : public Instruction {
public:
    LogicInst(BasicBlock *ownBB, InstId id, Opcode op, InstProxyList inputs, CmpFlags flags)
        : Instruction(ownBB, id, op, ResultType::BOOL, inputs), flags_(flags)
    {
        ASSERT(flags != CmpFlags::INVALID);
        ASSERT(inputs.size() == 2);
    }

    CmpFlags GetFlags() const
    {
        return flags_;
    }

    void Dump(std::stringstream &ss) const override;

private:
    CmpFlags flags_;
};

class BranchInst : public Instruction {
public:
    BranchInst(BasicBlock *ownBB, InstId id, Opcode op, InstProxyList inputs)
        : Instruction(ownBB, id, op, ResultType::VOID, inputs)
    {
        ASSERT((op == Opcode::BRANCH && inputs.size() == 0) || inputs.size() == 1);
    }

    void Dump(std::stringstream &ss) const override;
};

class ReturnInst : public Instruction {
public:
    ReturnInst(BasicBlock *ownBB, InstId id, Opcode op, ResultType resType, InstProxyList inputs)
        : Instruction(ownBB, id, op, resType, inputs)
    {
        ASSERT((resType == ResultType::VOID && inputs.size() == 0) || inputs.size() == 1);
    }

    void Dump(std::stringstream &ss) const override;
};

class PhiInst : public Instruction {
public:
    using ValueDependencies = std::unordered_multimap<Instruction *, BasicBlock *>;

    PhiInst(BasicBlock *ownBB, InstId id, ResultType resType) : Instruction(ownBB, id, Opcode::PHI, resType)
    {
        ASSERT(resType != ResultType::VOID);
    }

    void ResolveDependency(Instruction *value, BasicBlock *bb)
    {
        ASSERT(value->GetResultType() == GetResultType());
        valueDeps_.insert(std::pair {value, bb});
    }

    const ValueDependencies &GetValueDependencies()
    {
        return valueDeps_;
    }

    void Dump(std::stringstream &ss) const override;

private:
    ValueDependencies valueDeps_;
};

}  // namespace compiler::ir

#endif  // IR_INSTRUCTION_H
