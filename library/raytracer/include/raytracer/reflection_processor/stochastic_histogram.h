#pragma once

namespace raytracer {
namespace reflection_processor {

class stochastic_histogram final {
public:
    /// A max_image_source_order of 0 = direct energy from image-source
    /// An order of 1 = direct and one reflection from image-source
    /// i.e. the order == the number of reflections for each image
    stochastic_histogram(const compute_context& cc,
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
        if (max_image_source_order_ <= step) {
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

class make_stochastic_histogram final {
public:
    make_stochastic_histogram(float receiver_radius,
                              float sample_rate,
                              size_t max_order)
            : receiver_radius_{receiver_radius}
            , sample_rate_{sample_rate}
            , max_order_{max_order} {}

    auto operator()(const compute_context& cc,
                    const model::parameters& params,
                    const voxelised_scene_data<cl_float3, surface>& voxelised,
                    size_t num_directions) const {
        return stochastic_histogram{cc,
                                    params,
                                    receiver_radius_,
                                    sample_rate_,
                                    max_order_,
                                    num_directions};
    }

private:
    float receiver_radius_;
    float sample_rate_;
    size_t max_order_;
};

}  // namespace reflection_processor
}  // namespace raytracer
