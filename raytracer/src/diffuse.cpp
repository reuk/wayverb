#include "raytracer/diffuse.h"

namespace raytracer {

diffuse_finder::diffuse_finder(const cl::Context& context,
                               const cl::Device& device,
                               size_t rays,
                               size_t depth)
        : context(context)
        , device(device)
        , impulse_builder(rays, depth) {}

void diffuse_finder::push(const aligned::vector<Reflection>& reflections) {
    //  TODO
    impulse_builder.push(aligned::vector<Impulse>(reflections.size()));
}

const aligned::vector<aligned::vector<Impulse>>& diffuse_finder::get_results()
        const {
    return impulse_builder.get_data();
}

aligned::vector<aligned::vector<Impulse>>& diffuse_finder::get_results() {
    return impulse_builder.get_data();
}

}  // namespace raytracer
