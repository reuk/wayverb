#pragma once

#include "vec.h"
#include "config.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/error/en.h"
#include "rapidjson/document.h"

#include <vector>
#include <map>

class CuboidBoundary;
class MeshBoundary;

using VolumeType = cl_float8;

typedef struct {
    cl_ulong surface;
    cl_ulong v0;
    cl_ulong v1;
    cl_ulong v2;
} __attribute__((aligned(8))) Triangle;

typedef struct {
    VolumeType specular;
    VolumeType diffuse;
} __attribute__((aligned(8))) Surface;

using TriangleVec3f = std::array<Vec3f, 3>;
TriangleVec3f get_triangle_verts(const Triangle& t,
                                 const std::vector<Vec3f>& v);
TriangleVec3f get_triangle_verts(const Triangle& t,
                                 const std::vector<cl_float3>& v);

class SurfaceLoader {
public:
    using size_type = std::vector<Surface>::size_type;

    SurfaceLoader(const std::string& fpath);

    std::vector<Surface> get_surfaces() const;
    size_type get_index(const std::string& name) const;

private:
    void add_surface(const std::string& name, const Surface& surface);

    std::vector<Surface> surfaces;
    std::map<std::string, size_type> surface_indices;
};

class SceneData {
public:
    using size_type = std::vector<Surface>::size_type;

    SceneData(const std::string& fpath,
              const std::string& mat_file,
              float scale = 1);
    SceneData(const aiScene* const scene,
              const std::string& mat_file,
              float scale = 1);
    virtual ~SceneData() noexcept = default;

    void populate(const aiScene* const scene,
                  const std::string& mat_file,
                  float scale);
    void populate(const std::string& fpath,
                  const std::string& mat_file,
                  float scale);

    CuboidBoundary get_aabb() const;
    std::vector<Vec3f> get_converted_vertices() const;
    std::vector<int> get_triangle_indices() const;

    const std::vector<Triangle>& get_triangles() const;
    const std::vector<cl_float3>& get_vertices() const;
    const std::vector<Surface>& get_surfaces() const;

private:
    std::vector<Triangle> triangles;
    std::vector<cl_float3> vertices;
    std::vector<Surface> surfaces;
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
