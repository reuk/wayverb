#include "common/scene_data.h"
#include "common/conversions.h"

#include "utilities/map_to_vector.h"

aligned::vector<glm::vec3> convert(const aligned::vector<cl_float3>& c) {
    return map_to_vector(
            begin(c), end(c), [](const auto& i) { return to_vec3(i); });
}
