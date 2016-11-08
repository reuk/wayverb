#include "combined/model/waveguide.h"

#include "utilities/range.h"

namespace wayverb {
namespace combined {
namespace model {

void single_band_waveguide::set_cutoff(double cutoff) {
    data_.cutoff = cutoff;
    notify();
}

void single_band_waveguide::set_usable_portion(double usable) {
    data_.usable_portion = clamp(usable, util::make_range(0.0, 1.0));
    notify();
}

wayverb::waveguide::single_band_parameters single_band_waveguide::get() const {
    return data_;
}

////////////////////////////////////////////////////////////////////////////////

void multiple_band_waveguide::set_bands(size_t bands) {
    data_.bands = clamp(bands, util::make_range(size_t{1}, size_t{8}));
    maintain_valid_cutoff();
    notify();
}

void multiple_band_waveguide::set_cutoff(double cutoff) {
    data_.cutoff = cutoff;
    maintain_valid_cutoff();
    notify();
}

void multiple_band_waveguide::set_usable_portion(double usable) {
    data_.usable_portion = usable;
    notify();
}

wayverb::waveguide::multiple_band_constant_spacing_parameters
multiple_band_waveguide::get() const {
    return data_;
}

void multiple_band_waveguide::maintain_valid_cutoff() {
    data_.cutoff = std::max(data_.cutoff, band_params_.edges[data_.bands]);
}

////////////////////////////////////////////////////////////////////////////////

waveguide::waveguide() { connect_all(single_band, multiple_band); }

void waveguide::set_mode(mode mode) {
    mode_ = mode;
    notify();
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
