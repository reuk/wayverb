#include "combined/model/scene.h"

namespace wayverb {
namespace combined {
namespace model {

scene::scene(core::geo::box aabb)
        : aabb_{std::move(aabb)} {
    add_source(0);
    add_receiver(0);
    connect(raytracer, waveguide, output, sources_, receivers_);
}

const class source& scene::source(size_t index) const {
    return sources_[index];
}

class source& scene::source(size_t index) {
    return sources_[index];
}

void scene::add_source(size_t index) {
    class source tmp {
        aabb_
    };
    sources_.insert(sources_.begin() + index, std::move(tmp));
}

void scene::remove_source(size_t index) {
    if (1 < sources_.size()) {
        sources_.erase(sources_.begin() + index);
    }
}

const class receiver& scene::receiver(size_t index) const {
    return receivers_[index];
}

class receiver& scene::receiver(size_t index) {
    return receivers_[index];
}

void scene::add_receiver(size_t index) {
    class receiver tmp {
        aabb_
    };
    receivers_.insert(receivers_.begin() + index, std::move(tmp));
}

void scene::remove_receiver(size_t index) {
    if (1 < receivers_.size()) {
        receivers_.erase(receivers_.begin() + index);
    }
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
