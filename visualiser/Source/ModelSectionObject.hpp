#pragma once

#include "BasicDrawableObject.hpp"
#include "BoxObject.hpp"

#include "scene_data.h"
#include "octree.h"

class ModelSectionObject final : public BasicDrawableObject {
public:
    ModelSectionObject(const GenericShader& shader,
                       const SceneData& scene_data,
                       const Octree& octree);
    void draw() const override;

private:
    std::vector<glm::vec3> get_vertices(const SceneData& scene_data) const;
    std::vector<GLuint> get_indices(const SceneData& scene_data,
                                    const Octree& octree) const;

    void draw_octree(const Octree& octree, BoxObject& box) const;

    Octree octree;
};
