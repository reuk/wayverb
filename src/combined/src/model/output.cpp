#include "combined/model/output.h"

namespace wayverb {
namespace combined {
namespace model {

void output::set_sample_rate(sample_rate sample_rate) {
    sample_rate_ = sample_rate;
    notify();
}

output::sample_rate output::get_sample_rate() const { return sample_rate_; }

void output::set_bit_depth(audio_file::bit_depth bit_depth) {
    bit_depth_ = bit_depth;
    notify();
}

audio_file::bit_depth output::get_bit_depth() const { return bit_depth_; }

void output::set_format(audio_file::format format) {
    format_ = format;
    notify();
}

audio_file::format output::get_format() const { return format_; }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
