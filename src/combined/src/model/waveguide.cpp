#include "combined/model/waveguide.h"

#include "waveguide/simulation_parameters.h"

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

const frequency_domain::edges_and_width_factor<9>
        multiple_band_waveguide::band_params_ =
                hrtf_data::hrtf_band_params_hz();

////////////////////////////////////////////////////////////////////////////////

void waveguide::set_mode(mode mode) {
    mode_ = mode;
    notify();
}

waveguide::mode waveguide::get_mode() const { return mode_; }

double waveguide::get_sampling_frequency() const {
    switch (mode_) {
        case mode::single:
            return wayverb::waveguide::compute_sampling_frequency(
                    get<single_band_waveguide>()->get());
        case mode::multiple:
            return wayverb::waveguide::compute_sampling_frequency(
                    get<multiple_band_waveguide>()->get());
    }
}

shared_value<single_band_waveguide>& waveguide::single_band() {
    return get<single_band_t>();
}
const shared_value<single_band_waveguide>& waveguide::single_band() const {
    return get<single_band_t>();
}

shared_value<multiple_band_waveguide>& waveguide::multiple_band() {
    return get<multiple_band_t>();
}
const shared_value<multiple_band_waveguide>& waveguide::multiple_band() const {
    return get<multiple_band_t>();
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
