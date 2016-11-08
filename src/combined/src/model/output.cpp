#include "combined/model/output.h"

namespace wayverb {
namespace combined {
namespace model {

void output::set_output_folder(std::string output_folder) {
    output_folder_ = std::move(output_folder);
    notify();
}

void output::set_name(std::string name) {
    name_ = std::move(name);
    notify();
}

void output::set_sample_rate(double sr) {
    sample_rate_ = sr;
    notify();
}

void output::set_bit_depth(bit_depth bit_depth) {
    bit_depth_ = bit_depth;
    notify();
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
