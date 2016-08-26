#include "common/scene_data.h"

#include "common/map_to_vector.h"
#include "common/stl_wrappers.h"

aligned::vector<glm::vec3> convert(const aligned::vector<cl_float3>& c) {
    return map_to_vector(c, [](const auto& i) { return to_vec3(i); });
}

//----------------------------------------------------------------------------//

scene_data::scene_data(aligned::vector<triangle> triangles,
                       aligned::vector<cl_float3> vertices,
                       aligned::vector<material> materials)
        : contents{std::move(triangles),
                   std::move(vertices),
                   std::move(materials)} {}

scene_data::scene_data(struct contents c)
        : contents(std::move(c)) {}

geo::box scene_data::get_aabb() const {
    const auto v = convert(get_vertices());
    return util::min_max(std::begin(v), std::end(v));
}

aligned::vector<size_t> scene_data::compute_triangle_indices() const {
    aligned::vector<size_t> ret(get_triangles().size());
    proc::iota(ret, 0);
    return ret;
}

const aligned::vector<triangle>& scene_data::get_triangles() const {
    return contents.triangles;
}
const aligned::vector<cl_float3>& scene_data::get_vertices() const {
    return contents.vertices;
}
const aligned::vector<scene_data::material>& scene_data::get_materials() const {
    return contents.materials;
}

aligned::vector<surface> scene_data::get_surfaces() const {
    aligned::vector<surface> ret(get_materials().size());
    proc::transform(get_materials(), ret.begin(), [](const auto& i) {
        return i.surface;
    });
    return ret;
}

void scene_data::set_surfaces(const aligned::vector<material>& materials) {
    for (const auto& i : materials) {
        set_surface(i);
    }
}

void scene_data::set_surfaces(
        const aligned::map<std::string, surface>& surfaces) {
    for (auto& i : surfaces) {
        set_surface(material{i.first, i.second});
    }
}

void scene_data::set_surface(const material& material) {
    auto it = proc::find_if(contents.materials, [&material](const auto& i) {
        return i.name == material.name;
    });
    if (it != get_materials().end()) {
        it->surface = material.surface;
    }
}

void scene_data::set_surfaces(const surface& surface) {
    proc::for_each(contents.materials,
                   [surface](auto& i) { i.surface = surface; });
}

void scene_data::set_contents(struct contents c) { contents = std::move(c); }
