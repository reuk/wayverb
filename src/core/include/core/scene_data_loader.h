#pragma once

#include "core/scene_data.h"

#include "utilities/aligned/unordered_map.h"
#include "utilities/map_to_vector.h"

#include <experimental/optional>

namespace wayverb {
namespace core {

class scene_data_loader final {
public:
    scene_data_loader();
    scene_data_loader(const std::string& fpath);

    //  need to declare but not define these here because pimpl idiom wew
    scene_data_loader(scene_data_loader&&) noexcept;
    scene_data_loader& operator=(scene_data_loader&&) noexcept;
    ~scene_data_loader() noexcept;

    void load(const std::string& f);
    void save(const std::string& f) const;

    void clear();

    /// Get all file extensions that the loader might understand.
    std::string get_extensions() const;

    using scene_data = generic_scene_data<cl_float3, std::string>;
    const std::experimental::optional<scene_data>& get_scene_data() const;

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

template <typename Vertex, typename Surface>
auto scene_with_extracted_surfaces(
        const generic_scene_data<Vertex, std::string>& scene,
        const util::aligned::unordered_map<std::string, Surface>&
                surface_table) {
    const auto surfaces = util::map_to_vector(
            begin(scene.get_surfaces()),
            end(scene.get_surfaces()),
            [&](const auto& i) {
                const auto it = surface_table.find(i);
                return it != surface_table.end() ? it->second : Surface();
            });

    return make_scene_data(
            scene.get_triangles(), scene.get_vertices(), std::move(surfaces));
}

}  // namespace core
}  // namespace wayverb
