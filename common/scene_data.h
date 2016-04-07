#pragma once

#include "config.h"
#include "vec.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/rapidjson.h"

#include <map>
#include <vector>

class CuboidBoundary;
class MeshBoundary;

typedef struct {
    cl_ulong surface;
    cl_ulong v0;
    cl_ulong v1;
    cl_ulong v2;
} __attribute__((aligned(8))) Triangle;

using VolumeType = cl_float8;

std::ostream& operator<<(std::ostream& os, const VolumeType& f);

typedef struct {
    VolumeType specular{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
    VolumeType diffuse{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
} __attribute__((aligned(8))) Surface;

constexpr bool validate_surface(const Surface& s,
                                float min_gain = 0.001,
                                float max_gain = 0.999) {
    bool ret = true;
    for (auto i = 0; i != sizeof(VolumeType) / sizeof(cl_float); ++i) {
        ret = ret && min_gain <= s.specular.s[i] &&
              s.specular.s[i] <= max_gain && min_gain <= s.diffuse.s[i] &&
              s.diffuse.s[i] <= max_gain;
    }
    return ret;
}

std::ostream& operator<<(std::ostream& os, const Surface& s);

using TriangleVec3f = std::array<Vec3f, 3>;
TriangleVec3f get_triangle_verts(const Triangle& t,
                                 const std::vector<Vec3f>& v);
TriangleVec3f get_triangle_verts(const Triangle& t,
                                 const std::vector<cl_float3>& v);

class SurfaceOwner;

class SurfaceLoader {
public:
    using size_type = std::vector<Surface>::size_type;

    SurfaceLoader(const std::string& fpath);

    const std::vector<Surface>& get_surfaces() const;
    size_type get_index(const std::string& name) const;

private:
    void add_surface(const std::string& name, const Surface& surface);

    std::vector<Surface> surfaces;
    std::map<std::string, size_type> surface_indices;

    friend class SurfaceOwner;
};

class SurfaceOwner {
public:
    SurfaceOwner(const std::vector<Surface>& surfaces);
    SurfaceOwner(std::vector<Surface>&& surfaces);
    SurfaceOwner(const SurfaceLoader& surface_loader);
    SurfaceOwner(SurfaceLoader&& surface_loader);

    const std::vector<Surface>& get_surfaces() const;

private:
    void check_num_surfaces() const;
    std::vector<Surface> surfaces;
};

class SceneData : public SurfaceOwner {
public:
    using size_type = std::vector<Surface>::size_type;

    SceneData(const std::string& fpath,
              const std::string& mat_file,
              float scale = 1);
    SceneData(const aiScene* const scene,
              const std::string& mat_file,
              float scale = 1);
    SceneData(const std::vector<Triangle>& triangles,
              const std::vector<cl_float3>& vertices,
              const std::vector<Surface>& surfaces);
    SceneData(std::vector<Triangle>&& triangles,
              std::vector<cl_float3>&& vertices,
              std::vector<Surface>&& surfaces);
    virtual ~SceneData() noexcept = default;

    CuboidBoundary get_aabb() const;
    std::vector<Vec3f> get_converted_vertices() const;
    std::vector<int> get_triangle_indices() const;

    const std::vector<Triangle>& get_triangles() const;
    const std::vector<cl_float3>& get_vertices() const;

private:
    SceneData(const aiScene* const scene,
              const SurfaceLoader& loader,
              float scale = 1);

    struct Contents {
        std::vector<Triangle> triangles;
        std::vector<cl_float3> vertices;
        std::vector<Surface> surfaces;
    };

    SceneData(const Contents& contents);
    SceneData(Contents&& contents);

    static Contents get_contents(const aiScene* const scene,
                                 const SurfaceLoader& loader,
                                 float scale);

    std::vector<Triangle> triangles;
    std::vector<cl_float3> vertices;
};

template <>
struct JsonGetter<Surface> {
    JsonGetter(Surface& t)
            : t(t) {
    }
    virtual ~JsonGetter() noexcept = default;

    /// Returns true if value is a json object.
    virtual bool check(const rapidjson::Value& value) const {
        return value.IsObject();
    }

    /// Attempts to run a ConfigValidator on value
    virtual void get(const rapidjson::Value& value) const {
        ConfigValidator cv;

        cv.addRequiredValidator("specular", t.specular);
        cv.addRequiredValidator("diffuse", t.diffuse);

        cv.run(value);
    }
    Surface& t;
};

void attemptJsonParse(const std::string& fname, rapidjson::Document& doc);
