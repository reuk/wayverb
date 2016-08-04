#pragma once

#include "common/aligned/map.h"
#include "common/aligned/vector.h"
#include "triangle.h"

#include "cl_common.h"

#include "glm/glm.hpp"

#include <map>
#include <vector>

using volume_type = cl_float8;

struct alignas(1 << 5) surface {
    volume_type specular{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
    volume_type diffuse{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
};

class box;
class mesh_boundary;
struct triangle;

//----------------------------------------------------------------------------//

class copyable_scene_data {
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

    copyable_scene_data() = default;
    copyable_scene_data(const aligned::vector<triangle>& triangles,
                        const aligned::vector<cl_float3>& vertices,
                        const aligned::vector<material>& materials);
    copyable_scene_data(contents&& rhs);

    copyable_scene_data(const copyable_scene_data& rhs) = default;
    copyable_scene_data& operator=(const copyable_scene_data& rhs) = default;
    copyable_scene_data(copyable_scene_data&& rhs) noexcept = default;
    copyable_scene_data& operator=(copyable_scene_data&& rhs) noexcept =
            default;
    virtual ~copyable_scene_data() noexcept = default;

    aligned::vector<surface> get_surfaces() const;
    void set_surfaces(const aligned::vector<material>& materials);
    void set_surfaces(const aligned::map<std::string, surface>& surfaces);
    void set_surface(const material& material);

    void set_surfaces(const surface& surface);

    box get_aabb() const;
    aligned::vector<glm::vec3> get_converted_vertices() const;
    aligned::vector<size_t> get_triangle_indices() const;

    const aligned::vector<triangle>& get_triangles() const;
    const aligned::vector<cl_float3>& get_vertices() const;
    const aligned::vector<material>& get_materials() const;

private:
    contents contents;
};

//----------------------------------------------------------------------------//

class scene_data final : public copyable_scene_data {
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    //  this class adds the ability to load/save from file
    scene_data(const std::string& fpath);
    void save(const std::string& f) const;

    scene_data(const scene_data& rhs) = delete;
    scene_data& operator=(const scene_data& rhs) = delete;
    scene_data(scene_data&& rhs) noexcept;
    scene_data& operator=(scene_data&& rhs) noexcept;
    ~scene_data() noexcept;

private:
    scene_data(copyable_scene_data&& rhs, std::unique_ptr<impl>&& pimpl);
    scene_data(std::tuple<copyable_scene_data, std::unique_ptr<impl>>&& rhs);
    static std::tuple<copyable_scene_data, std::unique_ptr<impl>> load(
            const std::string& scene_file);
};
