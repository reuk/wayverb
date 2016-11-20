#pragma once

#include "core/cl/triangle.h"

#include "utilities/aligned/vector.h"

#include <numeric>

namespace wayverb {
namespace core {

template <typename Vertex, typename Surface>
class generic_scene_data final {
public:
    using vertex_type = Vertex;
    using surface_type = Surface;

    generic_scene_data() = default;
    generic_scene_data(util::aligned::vector<triangle> triangles,
                       util::aligned::vector<vertex_type> vertices,
                       util::aligned::vector<surface_type> surfaces)
            : triangles_{triangles}
            , vertices_{vertices}
            , surfaces_{surfaces} {
        const auto check_surface = [&](auto s) {
            if (s < 0 || surfaces_.size() <= s) {
                throw std::runtime_error{"Surface index is out of range."};
            }
        };
        const auto check_vertex = [&](auto v) {
            if (v < 0 || vertices_.size() <= v) {
                throw std::runtime_error{"Vertex index is out of range."};
            }
        };

        for (const auto& tri : triangles_) {
            check_surface(tri.surface);
            check_vertex(tri.v0);
            check_vertex(tri.v1);
            check_vertex(tri.v2);
        }
    }

    const util::aligned::vector<triangle>& get_triangles() const {
        return triangles_;
    }
    const util::aligned::vector<vertex_type>& get_vertices() const {
        return vertices_;
    }
    const util::aligned::vector<surface_type>& get_surfaces() const {
        return surfaces_;
    }

    void swap(generic_scene_data& rhs) noexcept {
        using std::swap;
        swap(triangles_, rhs.triangles_);
        swap(vertices_, rhs.vertices_);
        swap(surfaces_, rhs.surfaces_);
    }

    /// This looks dumb, but the idea is to maintain the strong exception
    /// safety guarantee.
    template <typename It>
    void set_surfaces(It begin, It end) {
        generic_scene_data copy{
                triangles_,
                vertices_,
                util::aligned::vector<surface_type>{begin, end}};
        swap(copy);
    }

    /// Strongly exception safe as long as surface_type has a nothrow copy
    /// constructor, which it should do if it is POD.
    void set_surfaces(const surface_type& surface) {
        std::fill(surfaces_.begin(), surfaces_.end(), surface);
    }

private:
    util::aligned::vector<triangle> triangles_;
    util::aligned::vector<vertex_type> vertices_;
    util::aligned::vector<surface_type> surfaces_;
};

template <typename Vertex, typename Surface>
auto make_scene_data(util::aligned::vector<triangle> triangles,
                     util::aligned::vector<Vertex> vertices,
                     util::aligned::vector<Surface> surfaces) {
    return generic_scene_data<Vertex, Surface>{
            std::move(triangles), std::move(vertices), std::move(surfaces)};
}

}  // namespace core
}  // namespace wayverb
