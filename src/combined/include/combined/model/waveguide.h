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

    void set_bands(size_t bands);
    void set_cutoff(double cutoff);
    void set_usable_portion(double usable);

    waveguide::multiple_band_constant_spacing_parameters get() const;

private:
    static const frequency_domain::edges_and_width_factor<9> band_params_;

    void maintain_valid_cutoff();

    waveguide::multiple_band_constant_spacing_parameters data_{2, 500, 0.6};
};

////////////////////////////////////////////////////////////////////////////////

class waveguide final : public member<waveguide,
                                      single_band_waveguide,
                                      multiple_band_waveguide> {
public:
    waveguide();

    waveguide(const waveguide& other);
    waveguide(waveguide&& other) noexcept;

    waveguide& operator=(const waveguide& other);
    waveguide& operator=(waveguide&& other) noexcept;

    void swap(waveguide& other) noexcept;

    enum class mode { single, multiple };

    void set_mode(mode mode);
    mode get_mode() const;

    single_band_waveguide single_band;
    multiple_band_waveguide multiple_band;

private:
    mode mode_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
