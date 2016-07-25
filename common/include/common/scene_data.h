#pragma once

#include "common/aligned/map.h"
#include "common/aligned/vector.h"
#include "triangle.h"

#include "cl_common.h"

#include "glm/glm.hpp"

#include <map>
#include <vector>

using VolumeType = cl_float8;

inline bool operator==(const VolumeType& a, const VolumeType& b) {
    return cl_equal(a, b);
}

inline bool operator!=(const VolumeType& a, const VolumeType& b) {
    return !(a == b);
}

struct alignas(1 << 5) Surface {
    VolumeType specular{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
    VolumeType diffuse{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
};

class CuboidBoundary;
class MeshBoundary;
struct Triangle;

//----------------------------------------------------------------------------//

class CopyableSceneData {
public:
    struct Material final {
        std::string name;
        Surface surface;
    };

    struct Contents final {
        aligned::vector<Triangle> triangles;
        aligned::vector<cl_float3> vertices;
        aligned::vector<Material> materials;
    };

    CopyableSceneData() = default;
    CopyableSceneData(const aligned::vector<Triangle>& triangles,
                      const aligned::vector<cl_float3>& vertices,
                      const aligned::vector<Material>& materials);
    CopyableSceneData(Contents&& rhs);

    CopyableSceneData(const CopyableSceneData& rhs) = default;
    CopyableSceneData& operator=(const CopyableSceneData& rhs) = default;
    CopyableSceneData(CopyableSceneData&& rhs) noexcept        = default;
    CopyableSceneData& operator=(CopyableSceneData&& rhs) noexcept = default;
    virtual ~CopyableSceneData() noexcept                          = default;

    aligned::vector<Surface> get_surfaces() const;
    void set_surfaces(const aligned::vector<Material>& materials);
    void set_surfaces(const aligned::map<std::string, Surface>& surfaces);
    void set_surface(const Material& material);

    void set_surfaces(const Surface& surface);

    CuboidBoundary get_aabb() const;
    aligned::vector<glm::vec3> get_converted_vertices() const;
    aligned::vector<size_t> get_triangle_indices() const;

    const aligned::vector<Triangle>& get_triangles() const;
    const aligned::vector<cl_float3>& get_vertices() const;
    const aligned::vector<Material>& get_materials() const;

private:
    Contents contents;
};

//----------------------------------------------------------------------------//

class SceneData final : public CopyableSceneData {
    struct Impl;
    std::unique_ptr<Impl> pimpl;

public:
    //  this class adds the ability to load/save from file
    SceneData(const std::string& fpath);
    void save(const std::string& f) const;

    SceneData(const SceneData& rhs) = delete;
    SceneData& operator=(const SceneData& rhs) = delete;
    SceneData(SceneData&& rhs) noexcept;
    SceneData& operator=(SceneData&& rhs) noexcept;
    ~SceneData() noexcept;

private:
    SceneData(CopyableSceneData&& rhs, std::unique_ptr<Impl>&& pimpl);
    SceneData(std::tuple<CopyableSceneData, std::unique_ptr<Impl>>&& rhs);
    static std::tuple<CopyableSceneData, std::unique_ptr<Impl>> load(
            const std::string& scene_file);
};
