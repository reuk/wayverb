#include "combined/model/waveguide.h"

#include "waveguide/simulation_parameters.h"

#include "utilities/range.h"

namespace wayverb {
namespace combined {
namespace model {

single_band_waveguide::single_band_waveguide(double cutoff,
                                             double usable_portion)
        : data_{cutoff, usable_portion} {}

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

bool operator==(const single_band_waveguide& a,
                const single_band_waveguide& b) {
    return a.get() == b.get();
}

bool operator!=(const single_band_waveguide& a,
                const single_band_waveguide& b) {
    return !(a == b);
}

////////////////////////////////////////////////////////////////////////////////

multiple_band_waveguide::multiple_band_waveguide(size_t bands,
                                                 double cutoff,
                                                 double usable_portion)
        : data_{bands, cutoff, usable_portion} {}

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

bool operator==(const multiple_band_waveguide& a,
                const multiple_band_waveguide& b) {
    return a.get() == b.get();
}

bool operator!=(const multiple_band_waveguide& a,
                const multiple_band_waveguide& b) {
    return !(a == b);
}

////////////////////////////////////////////////////////////////////////////////

waveguide::waveguide(mode mode,
                     single_band_waveguide single,
                     multiple_band_waveguide multiple)
        : base_type{std::move(single), std::move(multiple)}
        , mode_{mode} {}

waveguide::waveguide(single_band_waveguide single_band_waveguide)
        : base_type{std::move(single_band_waveguide), multiple_band_waveguide{}}
        , mode_{mode::single} {}

waveguide::waveguide(multiple_band_waveguide multiple_band_waveguide)
        : base_type{single_band_waveguide{}, std::move(multiple_band_waveguide)}
        , mode_{mode::multiple} {}

void waveguide::set_mode(mode mode) {
    mode_ = mode;
    notify();
}

waveguide::mode waveguide::get_mode() const { return mode_; }

bool operator==(const waveguide& a, const waveguide& b) {
    return static_cast<const waveguide::base_type&>(a) ==
                   static_cast<const waveguide::base_type&>(b) &&
           a.get_mode() == b.get_mode();
}

bool operator!=(const waveguide& a, const waveguide& b) { return !(a == b); }

////////////////////////////////////////////////////////////////////////////////

double compute_sampling_frequency(const waveguide& waveguide) {
    switch (waveguide.get_mode()) {
        case waveguide::mode::single:
            return compute_sampling_frequency(waveguide.single_band()->get());

        case waveguide::mode::multiple:
            return compute_sampling_frequency(waveguide.multiple_band()->get());
    }
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
