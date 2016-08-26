#pragma once

#include "common/aligned/map.h"
#include "common/aligned/vector.h"
#include "common/cl/common.h"
#include "common/cl/scene_structs.h"
#include "common/geo/box.h"

#include "glm/glm.hpp"

class mesh_boundary;
struct triangle;

class scene_data final {
public:
    struct material final {
        std::string name;
        surface surface;
    };

    struct contents final {
        aligned::vector<triangle> triangles;
        aligned::vector<cl_float3> vertices;
        aligned::vector<material> materials;
    };

    scene_data() = default;
    scene_data(aligned::vector<triangle> triangles,
               aligned::vector<cl_float3> vertices,
               aligned::vector<material> materials);
    scene_data(contents rhs);

    aligned::vector<surface> get_surfaces() const;
    void set_surfaces(const aligned::vector<material>& materials);
    void set_surfaces(const aligned::map<std::string, surface>& surfaces);
    void set_surface(const material& material);

    void set_surfaces(const surface& surface);

    geo::box get_aabb() const;
    aligned::vector<size_t> compute_triangle_indices() const;

    const aligned::vector<triangle>& get_triangles() const;
    const aligned::vector<cl_float3>& get_vertices() const;
    const aligned::vector<material>& get_materials() const;

    //  pass by value because we need a copy anyway
    void set_contents(contents c);

private:
    contents contents;
};

aligned::vector<glm::vec3> convert(const aligned::vector<cl_float3>& c);
