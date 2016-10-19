#pragma once

namespace raytracer {
namespace reflection_processor {

class image_source final {
public:
    image_source(const model::parameters& params,
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
        raytracer::image_source::tree tree{};
        for (const auto& path : builder_.get_data()) {
            tree.push(path);
        }
        //  fetch image source results
        auto ret{raytracer::image_source::postprocess<
                raytracer::image_source::fast_pressure_calculator<>>(
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

    raytracer::image_source::reflection_path_builder builder_;
};

class make_image_source final {
public:
    explicit make_image_source(size_t max_order)
            : max_order_{max_order} {}

    auto operator()(const compute_context& cc,
                    const model::parameters& params,
                    const voxelised_scene_data<cl_float3, surface>& voxelised,
                    size_t num_directions) const {
        return image_source{params, voxelised, max_order_, num_directions};
    }

private:
    size_t max_order_;
};

}  // namespace reflection_processor
}  // namespace raytracer
