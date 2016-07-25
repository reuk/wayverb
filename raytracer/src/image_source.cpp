#include "raytracer/image_source.h"

#include "common/aligned/set.h"

namespace raytracer {

namespace {

//  ray paths should be ordered by [ray][depth]
template <typename T>
aligned::set<aligned::vector<T>> compute_unique_paths(
        const aligned::vector<aligned::vector<T>>& path) {
    aligned::set<aligned::vector<T>> ret;

    //  for each ray
    for (auto j = 0; j != path.size(); ++j) {
        //  get all ray path combinations
        for (auto k = 1; k != path[j].size(); ++k) {
            aligned::vector<T> surfaces(path[j].begin(), path[j].begin() + k);

            //  add the path to the return set
            ret.insert(surfaces);
        }
    }

    return ret;
}

}  // namespace

//----------------------------------------------------------------------------//

image_source_finder::image_source_finder(size_t rays, size_t depth)
        : reflection_path_builder(rays, depth) {}

void image_source_finder::push(const aligned::vector<cl_ulong>& triangles) {
    reflection_path_builder.push(triangles);
}

aligned::vector<Impulse> image_source_finder::get_results() const {
    auto unique_paths =
            compute_unique_paths(reflection_path_builder.get_data());
    aligned::vector<Impulse> ret;
    //  TODO SUPER IMPORTANT IMAGE SOURCE CALCULATIONS
    return ret;
}

}  // namespace raytracer
