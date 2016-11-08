#pragma once

#include "combined/model/member.h"

#include "waveguide/simulation_parameters.h"

#include "hrtf/multiband.h"

namespace wayverb {
namespace combined {
namespace model {

class single_band_waveguide final : public member<single_band_waveguide> {
public:
    single_band_waveguide() = default;

    single_band_waveguide(const single_band_waveguide&) = delete;
    single_band_waveguide(single_band_waveguide&&) = delete;

    single_band_waveguide& operator=(const single_band_waveguide&) = delete;
    single_band_waveguide& operator=(single_band_waveguide&&) = delete;

    void set_cutoff(double cutoff);
    void set_usable_portion(double usable);

    waveguide::single_band_parameters get() const;

private:
    waveguide::single_band_parameters data_{500, 0.6};
};

////////////////////////////////////////////////////////////////////////////////

class multiple_band_waveguide final : public member<multiple_band_waveguide> {
public:
    multiple_band_waveguide() = default;

    multiple_band_waveguide(const multiple_band_waveguide&) = delete;
    multiple_band_waveguide(multiple_band_waveguide&&) noexcept = delete;

    multiple_band_waveguide& operator=(const multiple_band_waveguide&) = delete;
    multiple_band_waveguide& operator=(multiple_band_waveguide&&) noexcept =
            delete;

    void set_bands(size_t bands);
    void set_cutoff(double cutoff);
    void set_usable_portion(double usable);

    waveguide::multiple_band_constant_spacing_parameters get() const;

private:
    const frequency_domain::edges_and_width_factor<9> band_params_ =
            hrtf_data::hrtf_band_params_hz();

    void maintain_valid_cutoff();

    waveguide::multiple_band_constant_spacing_parameters data_{2, 500, 0.6};
};

////////////////////////////////////////////////////////////////////////////////

class waveguide final : public member<waveguide> {
public:
    waveguide();

    waveguide(const waveguide&) = delete;
    waveguide(waveguide&&) noexcept = delete;

    waveguide& operator=(const waveguide&) = delete;
    waveguide& operator=(waveguide&&) noexcept = delete;

    enum class mode { single, multiple };

    void set_mode(mode mode);

    single_band_waveguide single_band;
    multiple_band_waveguide multiple_band;

private:
    mode mode_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
