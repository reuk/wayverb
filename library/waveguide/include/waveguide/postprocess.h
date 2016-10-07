#pragma once

#include "waveguide/attenuator.h"

#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"
#include "common/cl/iterator.h"
#include "common/mixdown.h"
#include "common/model/receiver_settings.h"

#include "utilities/map_to_vector.h"

namespace waveguide {

template <typename It>
aligned::vector<aligned::vector<float>> attenuate_microphone(
        const model::receiver_settings& receiver,
        double acoustic_impedance,
        It begin,
        It end) {
    return map_to_vector(std::begin(receiver.microphones),
                         std::end(receiver.microphones),
                         [&](const auto& i) {
                             return attenuate(
                                     microphone{get_pointing(i.orientable,
                                                             receiver.position),
                                                i.shape},
                                     acoustic_impedance,
                                     begin,
                                     end);
                         });
}

template <typename It>
aligned::vector<aligned::vector<float>> attenuate_hrtf(
        const model::receiver_settings& receiver,
        double acoustic_impedance,
        double sample_rate,
        range<double> audible_range,
        It begin,
        It end) {
    const auto channels = {hrtf::channel::left, hrtf::channel::right};
    return map_to_vector(
            std::begin(channels), std::end(channels), [&](const auto& i) {
                auto attenuated{attenuate(
                        hrtf{get_pointing(receiver.hrtf, receiver.position),
                             glm::vec3{0, 1, 0},
                             i},
                        acoustic_impedance,
                        begin,
                        end)};
                constexpr auto bands{::detail::components_v<typename decltype(
                        attenuated)::value_type>};
                return multiband_filter_and_mixdown<bands>(
                        attenuated.begin(),
                        attenuated.end(),
                        audible_range,
                        [](auto it, auto index) {
                            return make_cl_type_iterator(std::move(it), index);
                        });
            });
}

//----------------------------------------------------------------------------//

template <typename It>
aligned::vector<aligned::vector<float>> run_attenuation(
        const model::receiver_settings& receiver,
        double acoustic_impedance,
        double sample_rate,
        range<double> audible_range,
        It begin,
        It end) {
    switch (receiver.mode) {
        case model::receiver_settings::mode::microphones:
            return attenuate_microphone(
                    receiver, acoustic_impedance, begin, end);
        case model::receiver_settings::mode::hrtf:
            return attenuate_hrtf(receiver,
                                  acoustic_impedance,
                                  sample_rate,
                                  audible_range,
                                  begin,
                                  end);
    }
}

}  // namespace waveguide
