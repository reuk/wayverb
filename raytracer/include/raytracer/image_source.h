#pragma once

#include "raytracer/cl_structs.h"
#include "raytracer/iterative_builder.h"

#include "common/aligned/set.h"
#include "common/aligned/vector.h"
#include "common/cl_include.h"

class VoxelCollection;

namespace raytracer {

class image_source_finder final {
public:
    image_source_finder(size_t rays, size_t depth);

    void push(const aligned::vector<cl_ulong>& triangles);
    aligned::vector<Impulse> get_results(const glm::vec3& source,
                                         const glm::vec3& receiver,
                                         const CopyableSceneData& scene_data,
                                         const VoxelCollection& vox) const;

private:
    iterative_builder<cl_ulong> reflection_path_builder;
};

}  // namespace raytracer
