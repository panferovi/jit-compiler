#ifndef IR_MARKER_H
#define IR_MARKER_H

#include "utils/macros.h"

#include <cstdint>

namespace compiler::ir {

class Marker {
public:
    explicit Marker() = default;

    explicit Marker(uint64_t value) : value_(value)
    {
        // is power of two
        ASSERT((value & (value - 1)) == 0);
    }

    void Mark(Marker marker)
    {
        value_ |= marker.value_;
    }

    void Unmark(Marker marker)
    {
        value_ &= ~marker.value_;
    }

    bool IsMarked(Marker marker) const
    {
        return (value_ & marker.value_) != 0;
    }

    bool IsEmpty() const
    {
        return value_ == 0;
    }

private:
    uint64_t value_ = 0;
};

}  // namespace compiler::ir

#endif  // IR_MARKER_H
