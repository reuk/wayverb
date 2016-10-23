#pragma once

#include "raytracer/attenuator.h"
#include "raytracer/cl/structs.h"
#include "raytracer/histogram.h"

#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"
#include "common/cl/common.h"
#include "common/cl/iterator.h"
#include "common/mixdown.h"
#include "common/model/receiver.h"
#include "common/pressure_intensity.h"
#include "common/scene_data.h"

#include "utilities/map_to_vector.h"

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
    return multiband_filter_and_mixdown(
            begin(hist), end(hist), sample_rate, [](auto it, auto index) {
                return make_cl_type_iterator(std::move(it), index);
            });
}

}  // namespace image_source
}  // namespace raytracer
