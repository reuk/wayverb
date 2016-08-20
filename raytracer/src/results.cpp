#include "raytracer/results.h"

namespace raytracer {

results::results(std::experimental::optional<impulse>&& direct,
                 aligned::vector<impulse>&& image_source,
                 aligned::vector<aligned::vector<impulse>>&& diffuse,
                 const glm::vec3& receiver,
                 double speed_of_sound)
        : direct(std::move(direct))
        , image_source(std::move(image_source))
        , diffuse(std::move(diffuse))
        , receiver(receiver)
        , speed_of_sound(speed_of_sound) {}

aligned::vector<impulse> results::get_impulses(bool use_direct,
                                               bool use_image_source,
                                               bool use_diffuse) const {
    const size_t direct_size       = use_direct ? 1 : 0;
    const size_t image_source_size = use_image_source ? image_source.size() : 0;
    const size_t diffuse_size =
            use_diffuse ? diffuse.size() * diffuse.front().size() : 0;

    aligned::vector<impulse> ret;
    ret.reserve(direct_size + image_source_size + diffuse_size);

    if (use_direct && direct) {
        ret.push_back(*direct);
    }

    if (use_image_source) {
        for (const auto& i : image_source) {
            ret.push_back(i);
        }
    }

    if (use_diffuse) {
        for (const auto& i : diffuse) {
            for (const auto& j : i) {
                ret.push_back(j);
            }
        }
    }

    return ret;
}

std::experimental::optional<impulse> results::get_direct() const {
    return direct;
}

aligned::vector<impulse> results::get_image_source() const {
    return image_source;
}

aligned::vector<aligned::vector<impulse>> results::get_diffuse() const {
    return diffuse;
}

glm::vec3 results::get_receiver() const { return receiver; }
double results::get_speed_of_sound() const { return speed_of_sound; }

}  // namespace raytracer
