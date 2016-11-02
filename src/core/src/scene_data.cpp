#include "core/scene_data.h"
#include "core/conversions.h"

#include "utilities/map_to_vector.h"

util::aligned::vector<glm::vec3> convert(
        const util::aligned::vector<cl_float3>& c) {
    return util::map_to_vector(
            begin(c), end(c), [](const auto& i) { return to_vec3(i); });
}
