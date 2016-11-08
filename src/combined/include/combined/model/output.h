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
    std::string get_output_folder() const;

    void set_name(std::string name);
    std::string get_name() const;

    void set_sample_rate(double sr);
    double get_sample_rate() const;

    void set_bit_depth(bit_depth bit_depth);
    bit_depth get_bit_depth() const;

    template <typename Archive>
    void load(Archive& archive) {
        archive(output_folder_, name_, sample_rate_, bit_depth_);
        notify();
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(output_folder_, name_, sample_rate_, bit_depth_);
    }

private:
    std::string output_folder_ = ".";
    std::string name_ = "sig";
    double sample_rate_ = 44100.0;
    bit_depth bit_depth_ = bit_depth::bd16;
};

constexpr auto convert_bit_depth(output::bit_depth m) {
    switch (m) {
        case output::bit_depth::bd16: return 16;
        case output::bit_depth::bd24: return 24;
    }
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
