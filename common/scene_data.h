#pragma once

#include "vec.h"
#include "config.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <vector>
#include <map>

using VolumeType = cl_float8;

typedef struct {
    cl_ulong surface;
    cl_ulong v0;
    cl_ulong v1;
    cl_ulong v2;
} __attribute__ ((aligned(8))) Triangle;

typedef struct  {
    VolumeType specular;
    VolumeType diffuse;
} __attribute__ ((aligned(8))) Surface;

class SurfaceLoader {
public:
    using size_type = std::vector<Surface>::size_type;

    SurfaceLoader(const std::string & fpath);

    std::vector<Surface> get_surfaces() const;
    size_type get_index(const std::string & name) const;
private:
    void add_surface(const std::string & name, const Surface & surface);

    std::vector<Surface> surfaces;
    std::map<std::string, size_type> surface_indices;
};

class SceneData {
public:
    using size_type = std::vector<Surface>::size_type;

    SceneData(const std::string & fpath, const std::string & mat_file);
    SceneData(const aiScene * const scene, const std::string & mat_file);
    virtual ~SceneData() noexcept = default;
    void populate(const aiScene * const scene, const std::string & mat_file);
    void populate(const std::string & fpath, const std::string & mat_file);

    std::vector<Triangle> triangles;
    std::vector<cl_float3> vertices;
    std::vector<Surface> surfaces;
};

template<>
struct JsonGetter<Surface>
{
    JsonGetter (Surface & t): t (t) {}

    /// Returns true if value is a json object.
    virtual bool check (const rapidjson::Value & value) const
    {
        return value.IsObject();
    }

    /// Attempts to run a ConfigValidator on value
    virtual void get (const rapidjson::Value & value) const
    {
        ConfigValidator cv;

        cv.addRequiredValidator ("specular", t.specular);
        cv.addRequiredValidator ("diffuse", t.diffuse);

        cv.run (value);
    }
    Surface & t;
};

