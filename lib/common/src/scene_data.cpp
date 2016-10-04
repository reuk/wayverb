#include "common/scene_data.h"

#include "common/conversions.h"
#include "common/map_to_vector.h"
#include "common/stl_wrappers.h"

aligned::vector<glm::vec3> convert(const aligned::vector<cl_float3>& c) {
    return map_to_vector(c, [](const auto& i) { return to_vec3(i); });
}
