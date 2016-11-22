#pragma once

#include "combined/model/member.h"

#include "waveguide/simulation_parameters.h"

#include "hrtf/multiband.h"

#include "cereal/types/base_class.hpp"

namespace wayverb {
namespace combined {
namespace model {

class single_band_waveguide final : public basic_member<single_band_waveguide> {
public:
    explicit single_band_waveguide(double cutoff = 500,
                                   double usable_portion = 0.6);

    void set_cutoff(double cutoff);
    void set_usable_portion(double usable);

    waveguide::single_band_parameters get() const;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(data_.cutoff, data_.usable_portion);
    }

    NOTIFYING_COPY_ASSIGN_DECLARATION(single_band_waveguide)
private:
    inline void swap(single_band_waveguide& other) noexcept {
        using std::swap;
        swap(data_, other.data_);
    };

    waveguide::single_band_parameters data_;
};

bool operator==(const single_band_waveguide& a, const single_band_waveguide& b);
bool operator!=(const single_band_waveguide& a, const single_band_waveguide& b);

////////////////////////////////////////////////////////////////////////////////

class multiple_band_waveguide final
        : public basic_member<multiple_band_waveguide> {
public:
    explicit multiple_band_waveguide(size_t bands = 2,
                                     double cutoff = 500,
                                     double usable_portion = 0.6);

    void set_bands(size_t bands);
    void set_cutoff(double cutoff);
    void set_usable_portion(double usable);

    waveguide::multiple_band_constant_spacing_parameters get() const;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(data_.bands, data_.cutoff, data_.usable_portion);
    }

    NOTIFYING_COPY_ASSIGN_DECLARATION(multiple_band_waveguide)
private:
    inline void swap(multiple_band_waveguide& other) noexcept {
        using std::swap;
        swap(data_, other.data_);
    };

    static const frequency_domain::edges_and_width_factor<9> band_params_;

    void maintain_valid_cutoff();

    waveguide::multiple_band_constant_spacing_parameters data_;
};

bool operator==(const multiple_band_waveguide& a,
                const multiple_band_waveguide& b);
bool operator!=(const multiple_band_waveguide& a,
                const multiple_band_waveguide& b);

////////////////////////////////////////////////////////////////////////////////

class waveguide final : public owning_member<waveguide,
                                             single_band_waveguide,
                                             multiple_band_waveguide> {
public:
    enum class mode { single, multiple };

    explicit waveguide(
            mode mode = mode::single,
            single_band_waveguide single = single_band_waveguide{},
            multiple_band_waveguide multiple = multiple_band_waveguide{});

    explicit waveguide(single_band_waveguide single_band_waveguide);
    explicit waveguide(multiple_band_waveguide multiple_band_waveguide);

    void set_mode(mode mode);
    mode get_mode() const;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::base_class<base_type>(this), mode_);
    }

    using single_band_t = single_band_waveguide;
    using multiple_band_t = multiple_band_waveguide;

    const auto& single_band() const { return get<0>(); }
    auto& single_band() { return get<0>(); }

    const auto& multiple_band() const { return get<1>(); }
    auto& multiple_band() { return get<1>(); }

    NOTIFYING_COPY_ASSIGN_DECLARATION(waveguide)
private:
    inline void swap(waveguide& other) noexcept {
        using std::swap;
        swap(mode_, other.mode_);
    };

    mode mode_ = mode::single;
};

bool operator==(const waveguide& a, const waveguide& b);
bool operator!=(const waveguide& a, const waveguide& b);

double compute_sampling_frequency(const waveguide& waveguide);

}  // namespace model
}  // namespace combined
}  // namespace wayverb
