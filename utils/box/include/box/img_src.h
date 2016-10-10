#pragma once

#include "raytracer/attenuator.h"
#include "raytracer/histogram.h"
#include "raytracer/cl/structs.h"

#include "common/pressure_intensity.h"

#include "common/geo/box.h"

template <typename Attenuator, typename It>
auto postprocess_impulses(It begin,
                          It end,
                          const glm::vec3& receiver,
                          const Attenuator& attenuator,
                          float speed_of_sound,
                          float acoustic_impedance,
                          float sample_rate) {
    //  Correct for directionality of the receiver.
    auto attenuated{raytracer::attenuate(attenuator, receiver, begin, end)};

    //  Correct for distance travelled.
    for (auto& it : attenuated) {
        it.volume *= pressure_for_distance(it.distance, acoustic_impedance);
    }

    //  Mix down to histogram.
    const auto histogram{raytracer::sinc_histogram(attenuated.begin(),
                                                   attenuated.end(),
                                                   speed_of_sound,
                                                   sample_rate,
                                                   20)};

    //  Extract.
    return mixdown(std::begin(histogram), std::end(histogram));
}

aligned::vector<impulse<8>> run_exact_img_src(const geo::box& box,
                                              float absorption,
                                              const glm::vec3& source,
                                              const glm::vec3& receiver,
                                              float speed_of_sound,
                                              float simulation_time);

aligned::vector<impulse<8>> run_fast_img_src(const geo::box& box,
                                             float absorption,
                                             const glm::vec3& source,
                                             const glm::vec3& receiver);
