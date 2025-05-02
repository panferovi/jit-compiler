#ifndef IR_ID_H
#define IR_ID_H

#include <cstdint>
#include <sstream>

namespace compiler::ir {

using Id = uint32_t;

class InstId {
public:
    explicit InstId(Id id, bool isPhi = false) : id_(id << PhiBit | isPhi) {}

    Id GetId() const
    {
        return id_ >> PhiBit;
    }

    bool IsPhi() const
    {
        return id_ & PhiBit;
    }

private:
    static constexpr uint32_t PhiBit = 1U;

    Id id_;
};

inline std::ostream &operator<<(std::ostream &os, const InstId &instId)
{
    os << instId.GetId() << (instId.IsPhi() ? "p" : "");
    return os;
}

}  // namespace compiler::ir

#endif  // IR_ID_H
