#pragma once

#include "raytracer/histogram.h"
#include "raytracer/stochastic/finder.h"
#include "raytracer/stochastic/postprocessing.h"

#include "core/attenuator/hrtf.h"
#include "core/spatial_division/scene_buffers.h"

namespace wayverb {

namespace raytracer {
namespace reflection_processor {

template <typename T, typename U, typename Alloc>
void energy_histogram_sum(const T& item,
                          double sample_rate,
                          std::vector<U, Alloc>& ret) {
    ret[time(item) * sample_rate] += volume(item);
}

template <typename T, typename U, typename Alloc, size_t Az, size_t El>
void energy_histogram_sum(
        const T& item,
        double sample_rate,
        core::vector_look_up_table<std::vector<U, Alloc>, Az, El>& ret) {
    using table = std::decay_t<decltype(ret)>;
    ret.at(table::index(item.pointing))[time(item) * sample_rate] +=
            volume(item);
}

struct energy_histogram_sum_functor final {
    template <typename T, typename Ret>
    void operator()(const T& item,
                    double sample_rate,
                    Ret& ret) const {
        energy_histogram_sum(item, sample_rate, ret);
    }
};

/// Where Histogram is probably a stochastic::energy_histogram or a
/// stochastic::directional_energy_histogram.
template <typename Histogram>
class stochastic_histogram final {
public:
    /// A max_image_source_order of 0 = direct energy from image-source
    /// An order of 1 = direct and one reflection from image-source
    /// i.e. the order == the number of reflections for each image
    stochastic_histogram(const core::compute_context& cc,
                         const core::model::parameters& params,
                         float receiver_radius,
                         float sample_rate,
                         size_t max_image_source_order,
                         size_t items)
            : finder_{cc, params, receiver_radius, items}
            , params_{params}
            , max_image_source_order_{max_image_source_order}
            , histogram_{sample_rate} {}

    template <typename It>
    void process(It b,
                 It e,
                 const core::scene_buffers& buffers,
                 size_t step,
                 size_t total) {
        const auto output = finder_.process(b, e, buffers);

        struct intermediate_impulse final {
            core::bands_type volume;
            double time;
            glm::vec3 pointing;
        };

        const auto intermediate = [&] {
            util::aligned::vector<intermediate_impulse> ret;
            ret.reserve(output.stochastic.size() + output.specular.size());

            const auto push_vector = [&](const auto& vec) {
                for (const auto& impulse : vec) {
                    ret.emplace_back(intermediate_impulse{
                            impulse.volume,
                            impulse.distance / params_.speed_of_sound,
                            glm::normalize(core::to_vec3(impulse.position) -
                                           params_.receiver)});
                }
            };

            push_vector(output.stochastic);
            if (max_image_source_order_ <= step) {
                push_vector(output.specular);
            }

            return ret;
        }();

        incremental_histogram(histogram_.histogram,
                              begin(intermediate),
                              end(intermediate),
                              histogram_.sample_rate,
                              energy_histogram_sum_functor{});
    }

    Histogram get_results() const { return histogram_; }

private:
    stochastic::finder finder_;
    core::model::parameters params_;
    size_t max_image_source_order_;
    Histogram histogram_;
};

class make_stochastic_histogram final {
public:
    make_stochastic_histogram(float receiver_radius,
                              float sample_rate,
                              size_t max_order);

    stochastic_histogram<stochastic::energy_histogram> operator()(
            const core::compute_context& cc,
            const core::model::parameters& params,
            const core::voxelised_scene_data<
                    cl_float3,
                    core::surface<core::simulation_bands>>& voxelised,
            size_t num_directions) const;

private:
    float receiver_radius_;
    float sample_rate_;
    size_t max_order_;
};

class make_directional_histogram final {
public:
    make_directional_histogram(float receiver_radius,
                               float sample_rate,
                               size_t max_order);

    stochastic_histogram<stochastic::directional_energy_histogram<20, 9>>
    operator()(const core::compute_context& cc,
               const core::model::parameters& params,
               const core::voxelised_scene_data<
                       cl_float3,
                       core::surface<core::simulation_bands>>& voxelised,
               size_t num_directions) const;

private:
    float receiver_radius_;
    float sample_rate_;
    size_t max_order_;
};

}  // namespace reflection_processor
}  // namespace raytracer
}  // namespace wayverb
