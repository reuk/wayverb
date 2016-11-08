#pragma once

#include "combined/model/member.h"

#include <string>

namespace wayverb {
namespace combined {
namespace model {

class output final : public member<output> {
public:
    output() = default;

    output(const output&) = default;
    output(output&&) noexcept = default;

    output& operator=(const output&) = default;
    output& operator=(output&&) noexcept = default;

    enum class bit_depth { bd16, bd24 };

    void set_output_folder(std::string output_folder);
    void set_name(std::string name);
    void set_sample_rate(double sr);
    void set_bit_depth(bit_depth bit_depth);

private:
    std::string output_folder_ = ".";
    std::string name_ = "sig";
    double sample_rate_ = 44100.0;
    bit_depth bit_depth_ = bit_depth::bd16;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
