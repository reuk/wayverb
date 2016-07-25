#include "raytracer/results.h"

namespace raytracer {

results::results(map_type&& image_source,
                 aligned::vector<aligned::vector<Impulse>>&& diffuse,
                 const glm::vec3& receiver,
                 const glm::vec3& source)
        : image_source(std::move(image_source))
        , diffuse(std::move(diffuse))
        , receiver(receiver)
        , source(source) {}

aligned::vector<Impulse> results::get_diffuse_impulses() const {
    aligned::vector<Impulse> ret;
    ret.reserve(diffuse.size() * diffuse.front().size());
    for (const auto& i : diffuse) {
        std::copy(i.begin(), i.end(), std::back_inserter(ret));
    }
    return ret;
}

aligned::vector<Impulse> results::get_image_source_impulses(
        bool remove_direct) const {
    auto temp = image_source;
    if (remove_direct) {
        auto it = std::find_if(temp.begin(), temp.end(), [](const auto& i) {
            return i.first == aligned::vector<cl_ulong>{0};
        });
        if (it != temp.end()) {
            temp.erase(it);
        }
    }

    aligned::vector<Impulse> ret;
    ret.reserve(temp.size());
    proc::transform(temp, std::back_inserter(ret), [](const auto& i) {
        return i.second;
    });
    return ret;
}

results::selected results::get_diffuse() const {
    return selected(get_diffuse_impulses(), receiver, source);
}

results::selected results::get_image_source(bool remove_direct) const {
    return selected(get_image_source_impulses(remove_direct), receiver, source);
}

results::selected results::get_all(bool remove_direct) const {
    auto diffuse     = get_diffuse_impulses();
    const auto image = get_image_source_impulses(remove_direct);
    diffuse.insert(diffuse.end(), image.begin(), image.end());
    return selected(diffuse, receiver, source);
}

results::selected::selected(const aligned::vector<Impulse>& impulses,
                            const glm::vec3& receiver,
                            const glm::vec3& source)
        : impulses(impulses)
        , receiver(receiver)
        , source(source) {}

aligned::vector<Impulse> results::selected::get_impulses() const {
    return impulses;
}
glm::vec3 results::selected::get_receiver() const { return receiver; }
glm::vec3 results::selected::get_source() const { return source; }

}  // namespace raytracer
