#include "waveguide/canonical.h"

namespace waveguide {

double compute_cutoff_frequency(const simulation_results& results) {
    if (!std::is_sorted(
                begin(results.bands), end(results.bands), [](auto i, auto j) {
                    return i.sample_rate < j.sample_rate;
                })) {
        throw std::runtime_error{
                "compute_cutoff_frequency: bands must be stored in order of "
                "increasing sample rate"};
    }
    return compute_cutoff_frequency(results.bands.back().sample_rate,
                                    results.usable_portion);
}

}  // namespace waveguide
