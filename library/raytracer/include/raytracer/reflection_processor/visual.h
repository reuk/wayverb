#pragma once

namespace raytracer {
namespace reflection_processor {

class visual final {
public:
    explicit visual(size_t items)
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

class make_visual final {
public:
    explicit make_visual(size_t items)
            : items_{items} {}

    auto operator()(const compute_context& cc,
                    const model::parameters& params,
                    const voxelised_scene_data<cl_float3, surface>& voxelised,
                    size_t num_directions) const {
        return visual{items_}; 
    }

private:
    size_t items_;
};

}  // namespace reflection_processor
}  // namespace raytracer
