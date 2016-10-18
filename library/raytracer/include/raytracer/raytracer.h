#pragma once

#include "raytracer/diffuse/finder.h"
#include "raytracer/diffuse/postprocessing.h"
#include "raytracer/get_direct.h"
#include "raytracer/image_source/finder.h"
#include "raytracer/image_source/reflection_path_builder.h"
#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflector.h"

#include "common/cl/common.h"
#include "common/cl/geometry.h"
#include "common/model/parameters.h"
#include "common/nan_checking.h"
#include "common/pressure_intensity.h"
#include "common/spatial_division/scene_buffers.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "utilities/apply.h"
#include "utilities/map.h"

#include <experimental/optional>
#include <iostream>

namespace raytracer {

struct results final {
    aligned::vector<impulse<8>> img_src;
    energy_histogram stochastic;
    aligned::vector<aligned::vector<reflection>> visual;
    model::parameters parameters;
};

class image_source_processor final {
public:
    image_source_processor(
            const model::parameters& params,
            const voxelised_scene_data<cl_float3, surface>& voxelised,
            size_t max_image_source_order,
            size_t items)
            : params_{params}
            , voxelised_{voxelised}
            , max_image_source_order_{max_image_source_order}
            , builder_{items} {}

    template <typename It>
    void process(It b,
                 It e,
                 const scene_buffers& buffers,
                 size_t step,
                 size_t total) {
        if (step < max_image_source_order_) {
            builder_.push(b, e);
        }
    }

    auto get_results() {
        image_source::tree tree{};
        for (const auto& path : builder_.get_data()) {
            tree.push(path);
        }
        //  fetch image source results
        auto ret{image_source::postprocess<
                image_source::fast_pressure_calculator<>>(
                begin(tree.get_branches()),
                end(tree.get_branches()),
                params_.source,
                params_.receiver,
                voxelised_,
                true)};

        if (const auto direct{
                    get_direct(params_.source, params_.receiver, voxelised_)}) {
            ret.emplace_back(*direct);
        }

        //  Correct for distance travelled.
        for (auto& imp : ret) {
            imp.volume *= pressure_for_distance(imp.distance,
                                                params_.acoustic_impedance);
        }

        return ret;
    }

private:
    model::parameters params_;
    const voxelised_scene_data<cl_float3, surface>& voxelised_;
    size_t max_image_source_order_;

    image_source::reflection_path_builder builder_;
};

class visual_processor final {
public:
    explicit visual_processor(size_t items)
            : builder_{items} {}

    template <typename It>
    void process(It b,
                 It e,
                 const scene_buffers& buffers,
                 size_t step,
                 size_t total) {
        builder_.push(b, b + builder_.get_num_items());
    }

    auto get_results() { return std::move(builder_.get_data()); }

private:
    iterative_builder<reflection> builder_;
};

class stochastic_processor final {
public:
    /// A max_image_source_order of 0 = direct energy from image-source
    /// An order of 1 = direct and one reflection from image-source
    /// i.e. the order == the number of reflections for each image
    stochastic_processor(const compute_context& cc,
                         const model::parameters& params,
                         float receiver_radius,
                         float sample_rate,
                         size_t max_image_source_order,
                         size_t items)
            : finder_{cc, params, receiver_radius, items}
            , params_{params}
            , sample_rate_{sample_rate}
            , max_image_source_order_{max_image_source_order} {}

    template <typename It>
    void process(It b,
                 It e,
                 const scene_buffers& buffers,
                 size_t step,
                 size_t total) {
        const auto output{finder_.process(b, e, buffers)};
        const auto to_histogram{[&](auto& in) {
            const auto make_iterator{[&](auto it) {
                return make_histogram_iterator(std::move(it),
                                               params_.speed_of_sound);
            }};
            constexpr auto max_time{60.0};
            incremental_histogram(histogram_,
                                  make_iterator(begin(in)),
                                  make_iterator(end(in)),
                                  sample_rate_,
                                  max_time,
                                  dirac_sum_functor{});
        }};
        to_histogram(output.diffuse);
        if (max_image_source_order_ < step) {
            to_histogram(output.specular);
        }
    }

    auto get_results() {
        return energy_histogram{std::move(histogram_), sample_rate_};
    }

private:
    diffuse::finder finder_;
    model::parameters params_;
    float sample_rate_;
    size_t max_image_source_order_;

    aligned::vector<volume_type> histogram_;
};

template <typename T>
class callback_wrapper_processor final {
public:
    explicit callback_wrapper_processor(T t)
            : t_{std::move(t)} {}

    template <typename It>
    void process(It b,
                 It e,
                 const scene_buffers& buffers,
                 size_t step,
                 size_t total) {
        t_(step, total);
    }

private:
    T t_;
};

template <typename T>
constexpr auto make_callback_wrapper_processor(T t) {
    return callback_wrapper_processor<T>{std::move(t)};
}

//----------------------------------------------------------------------------//

template <typename T>
class process_functor_adapter final {
public:
    explicit process_functor_adapter(T& t)
            : t_{t} {}

    template <typename... Ts>
    constexpr auto operator()(Ts&&... ts) {
        return t_.process(std::forward<Ts>(ts)...);        
    }

private:
    T& t_;
};

struct make_process_functor_adapter final {
    template <typename T>
    constexpr auto operator()(T& t) const {
        return process_functor_adapter<T>{t};
    }
};

//----------------------------------------------------------------------------//

template <typename It, typename PerStepCallback>
std::experimental::optional<results> run(
        It b_direction,
        It e_direction,
        const compute_context& cc,
        const voxelised_scene_data<cl_float3, surface>& voxelised,
        const model::parameters& params,
        float receiver_radius,
        size_t max_image_source_order,
        size_t rays_to_visualise,
        const std::atomic_bool& keep_going,
        const PerStepCallback& callback) {
    const size_t num_directions = std::distance(b_direction, e_direction);
    if (num_directions < rays_to_visualise) {
        throw std::runtime_error{
                "run: can't visualise more rays than will be traced"};
    }

    const scene_buffers buffers{cc.context, voxelised};

    image_source_processor image_source_processor{params,
                                                  voxelised,
                                                  max_image_source_order,
                                                  num_directions};
    stochastic_processor stochastic_processor{cc,
                                              params,
                                              receiver_radius,
                                              1000.0,
                                              max_image_source_order,
                                              num_directions};
    visual_processor visual_processor{rays_to_visualise};
    auto callback_wrapper_processor{make_callback_wrapper_processor(callback)};

    const auto make_ray_iterator{[&](auto it) {
        return make_mapping_iterator_adapter(std::move(it), [&](const auto& i) {
            return geo::ray{params.source, i};
        });
    }};

    reflector ref{cc,
                  params.receiver,
                  make_ray_iterator(b_direction),
                  make_ray_iterator(e_direction)};

    for (auto i{0ul},
         reflection_depth{
                 compute_optimum_reflection_number(voxelised.get_scene_data()),
         };
         i != reflection_depth;
         ++i) {
        if (!keep_going) {
            return std::experimental::nullopt;
        }

        const auto reflections{ref.run_step(buffers)};
        const auto b{begin(reflections)};
        const auto e{end(reflections)};
        call_each(map(std::tie(image_source_processor,
                               stochastic_processor,
                               visual_processor,
                               callback_wrapper_processor),
                      make_process_functor_adapter{}),
                  std::tie(b, e, buffers, i, reflection_depth));
    }

    return results{image_source_processor.get_results(),
                   stochastic_processor.get_results(),
                   visual_processor.get_results(),
                   params};
}

}  // namespace raytracer
