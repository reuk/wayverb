#include "combined/model/output.h"

namespace wayverb {
namespace combined {
namespace model {

void output::set_output_folder(std::string output_folder) {
    output_folder_ = std::move(output_folder);
    notify();
}

std::string output::get_output_folder() const { return output_folder_; }

void output::set_name(std::string name) {
    name_ = std::move(name);
    notify();
}

std::string output::get_name() const { return name_; }

void output::set_sample_rate(double sr) {
    sample_rate_ = sr;
    notify();
}

double output::get_sample_rate() const { return sample_rate_; }

void output::set_bit_depth(bit_depth bit_depth) {
    bit_depth_ = bit_depth;
    notify();
}

output::bit_depth output::get_bit_depth() const { return bit_depth_; }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
