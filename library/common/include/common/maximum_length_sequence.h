#pragma once

#include <functional>

using mls_callback = std::function<void(bool value, size_t step)>;

namespace {
constexpr uint32_t masks[] = {
        0x000000U, 0x000000U, 0x000002U, 0x000006U, 0x00000CU,
        0x000014U, 0x000030U, 0x000060U, 0x0000E1U, 0x000100U,
        0x000240U, 0x000500U, 0x000E08U, 0x001C80U, 0x003802U,
        0x006000U, 0x00D008U, 0x012000U, 0x020400U, 0x072000U,
        0x090000U, 0x500000U, 0xC00000U, 0x420000U, 0xE10000U};
}  // namespace

template <typename Word>
void generate_maximum_length_sequence(Word order,
                                      const mls_callback& callback) {
    const auto signal_length{(Word{1} << order) - 1};
    const auto mask{masks[order]};
    for (Word i{0ul}, reg{1}; i != signal_length; ++i) {
        callback(reg & 1, i);
        reg = (reg >> 1) ^ Word { (0 - (reg & 1)) & mask };
    }
}
