#pragma once

#include "common/scene_data.h"

#include "utilities/map_to_vector.h"

class scene_data_loader final {
public:
    struct material final {
        std::string name;
        surface<simulation_bands> surface;
    };

    scene_data_loader() = default;
    scene_data_loader(const std::string& fpath);

    //  need to declare but not define these here because pimpl idiom wew
    scene_data_loader(scene_data_loader&&) noexcept;
    scene_data_loader& operator=(scene_data_loader&&) noexcept;
    ~scene_data_loader() noexcept;

    void load(const std::string& f);
    void save(const std::string& f) const;

    bool is_loaded() const;
    void clear();

    using scene_data = generic_scene_data<cl_float3, material>;
    const scene_data& get_scene_data() const;

private:
    class impl;
    std::unique_ptr<impl> pimpl;
};

template <typename It>
auto extract_surfaces(It begin, It end) {
    return util::map_to_vector(begin, end, [](auto i) { return i.surface; });
}

template <typename Vertex, typename Surface>
auto scene_with_extracted_surfaces(
        const generic_scene_data<Vertex, Surface>& scene) {
    return make_scene_data(scene.get_triangles(),
                           scene.get_vertices(),
                           extract_surfaces(scene.get_surfaces().begin(),
                                            scene.get_surfaces().end()));
}
