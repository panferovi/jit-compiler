#ifndef UTILS_DEBUG_H
#define UTILS_DEBUG_H

namespace compiler::utils {
[[noreturn]] void AssertionFail(const char *expr, const char *file, unsigned line, const char *function);
}  // namespace compiler::utils

#endif  // UTILS_DEBUG_H
