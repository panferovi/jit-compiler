#ifndef IR_ID_H
#define IR_ID_H

#include <cstdint>
#include <sstream>

namespace compiler::ir {

using Id = uint32_t;

class InstId {
public:
    explicit InstId(Id id, bool isPhi = false) : id_(id << PHI_BIT | isPhi) {}

    Id GetId() const
    {
        return id_ >> PHI_BIT;
    }

    bool IsPhi() const
    {
        return id_ & PHI_BIT;
    }

private:
    static constexpr uint32_t PHI_BIT = 1U;

    Id id_;
};

inline std::ostream &operator<<(std::ostream &os, const InstId &instId)
{
    os << instId.GetId() << (instId.IsPhi() ? "p" : "");
    return os;
}

}  // namespace compiler::ir

#endif  // IR_ID_H
