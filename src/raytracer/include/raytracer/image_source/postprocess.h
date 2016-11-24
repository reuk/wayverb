#pragma once

#include "raytracer/attenuator.h"
#include "raytracer/cl/structs.h"
#include "raytracer/histogram.h"

#include "core/attenuator/hrtf.h"
#include "core/attenuator/microphone.h"
#include "core/cl/common.h"
#include "core/cl/traits.h"
#include "core/cl/iterator.h"
#include "core/mixdown.h"
#include "core/pressure_intensity.h"
#include "core/scene_data.h"

#include "utilities/map_to_vector.h"

namespace wayverb {
namespace raytracer {
namespace image_source {

template <typename InputIt, typename Method>
auto postprocess(InputIt b,
                 InputIt e,
                 const Method& method,
                 const glm::vec3& position,
                 double speed_of_sound,
                 double sample_rate) {
    const auto make_iterator = [&](auto it) {
        return make_histogram_iterator(
                make_attenuator_iterator(std::move(it), method, position),
                speed_of_sound);
    };
    auto hist = histogram(make_iterator(b),
                          make_iterator(e),
                          sample_rate,
                          sinc_sum_functor{});
    const auto m = std::accumulate(begin(hist), end(hist),
        typename decltype(hist)::value_type{},
        [](auto a, auto b) {
            return max(a, abs(b));
        });
    std::cout << "max: " << max_element(m) << '\n';
    return core::multiband_filter_and_mixdown(
            begin(hist), end(hist), sample_rate, [](auto it, auto index) {
                return core::make_cl_type_iterator(std::move(it), index);
            });
}

}  // namespace image_source
}  // namespace raytracer
}  // namespace wayverb
