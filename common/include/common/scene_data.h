#pragma once

#include "vec.h"

#include "cl_include.h"

#include <map>
#include <vector>

using VolumeType = cl_float8;

struct __attribute__((aligned(8))) Surface {
    VolumeType specular{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
    VolumeType diffuse{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
};

class CuboidBoundary;
class MeshBoundary;
struct Triangle;

struct aiScene;

class SurfaceConfig final {
public:
    SurfaceConfig() = default;
    explicit SurfaceConfig(const std::string& fpath);
    explicit SurfaceConfig(std::istream& stream);
    explicit SurfaceConfig(const std::map<std::string, Surface>& surfaces);

    void load(const std::string& fpath);
    void load(std::istream& stream);

    void save(const std::string& fpath);
    void save(std::ostream& stream);

    const std::map<std::string, Surface>& get_surfaces() const;

    template <typename Archive>
    void serialize(Archive& archive);

private:
    std::map<std::string, Surface> surfaces;
};

//----------------------------------------------------------------------------//

class SceneData {
public:
    struct Material {
        std::string name;
        Surface surface;
    };

    SceneData(const std::string& fpath,
              const std::string& mat,
              float scale = 1);
    SceneData(const std::string& fpath, float scale = 1);
    SceneData(const aiScene* const scene, float scale = 1);

    SceneData(const std::vector<Triangle>& triangles,
              const std::vector<cl_float3>& vertices,
              const std::vector<Material>& materials);

    std::vector<Surface> get_surfaces() const;
    void set_surfaces(const std::map<std::string, Surface>& surfaces);
    void set_surfaces(const SurfaceConfig& surfaces);
    void set_surface(const std::string& name, const Surface& surface);

    void set_surfaces(const Surface& surface);

    CuboidBoundary get_aabb() const;
    std::vector<Vec3f> get_converted_vertices() const;
    std::vector<int> get_triangle_indices() const;

    const std::vector<Triangle>& get_triangles() const;
    const std::vector<cl_float3>& get_vertices() const;
    const std::vector<Material>& get_materials() const;

private:
    struct Contents {
        std::vector<Triangle> triangles;
        std::vector<cl_float3> vertices;
        std::vector<Material> materials;
    };

    SceneData(const Contents& contents);

    static Contents load(const aiScene* const scene, float scale = 1);

    std::vector<Triangle> triangles;
    std::vector<cl_float3> vertices;
    std::vector<Material> materials;
};
