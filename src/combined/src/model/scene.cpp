#include "combined/model/scene.h"

namespace wayverb {
namespace combined {
namespace model {

scene::scene(core::geo::box aabb)
        : aabb_{std::move(aabb)}
        , sources{aabb_}
        , receivers{aabb_} {
    connect(sources, receivers, raytracer, waveguide, output);
}

void scene::swap(scene& other) noexcept {
    using std::swap;
    swap(sources, other.sources);
    swap(receivers, other.receivers);
    swap(raytracer, other.raytracer);
    swap(waveguide, other.waveguide);
    swap(output, other.output);
    swap(aabb_, other.aabb_);
}

scene::scene(const scene& other)
        : aabb_{other.aabb_} 
        , sources{other.sources}
        , receivers{other.receivers}
        , raytracer{other.raytracer}
        , waveguide{other.waveguide}
        , output{other.output} {
    connect(sources, receivers, raytracer, waveguide, output);
}

scene::scene(scene&& other) noexcept
        : sources{core::geo::box{}}
        , receivers{core::geo::box{}} {
    swap(other);
    connect(sources, receivers, raytracer, waveguide, output);
}

scene& scene::operator=(const scene& other) {
    auto copy{other};
    swap(copy);
    connect(sources, receivers, raytracer, waveguide, output);
    return *this;
}

scene& scene::operator=(scene&& other) noexcept {
    swap(other);
    connect(sources, receivers, raytracer, waveguide, output);
    return *this;
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
