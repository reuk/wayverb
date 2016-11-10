#include "combined/model/scene.h"

namespace wayverb {
namespace combined {
namespace model {

scene::scene(core::geo::box aabb)
        : type{sources_t{aabb},
               receivers_t{aabb},
               raytracer_t{},
               waveguide_t{},
               output_t{}} {}

sources& scene::sources() { return get<sources_t>(); }
const sources& scene::sources() const { return get<sources_t>(); }

receivers& scene::receivers() { return get<receivers_t>(); }
const receivers& scene::receivers() const { return get<receivers_t>(); }

raytracer& scene::raytracer() { return get<raytracer_t>(); }
const raytracer& scene::raytracer() const { return get<raytracer_t>(); }

waveguide& scene::waveguide() { return get<waveguide_t>(); }
const waveguide& scene::waveguide() const { return get<waveguide_t>(); }

output& scene::output() { return get<output_t>(); }
const output& scene::output() const { return get<output_t>(); }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
