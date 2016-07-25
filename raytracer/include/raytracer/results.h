#pragma once

#include "raytracer/cl_structs.h"

#include "common/aligned/map.h"
#include "common/aligned/vector.h"
#include "common/cl_include.h"

namespace raytracer {

class results final {
public:
    using map_type = aligned::map<aligned::vector<cl_ulong>, Impulse>;

    results(map_type&& image_source,
            aligned::vector<aligned::vector<Impulse>>&& diffuse,
            const glm::vec3& receiver,
            const glm::vec3& source);
    /// Raytraces are calculated in relation to a specific microphone position.
    /// This is a struct to keep the impulses and mic position together, because
    /// you'll probably never need one without the other.
    class selected {
    public:
        selected(const aligned::vector<Impulse>& impulses,
                 const glm::vec3& receiver,
                 const glm::vec3& source);

        aligned::vector<Impulse> get_impulses() const;
        glm::vec3 get_receiver() const;
        glm::vec3 get_source() const;

    private:
        aligned::vector<Impulse> impulses;
        glm::vec3 receiver;
        glm::vec3 source;
    };

    selected get_diffuse() const;
    selected get_image_source(bool remove_direct) const;
    selected get_all(bool remove_direct) const;

private:
    aligned::vector<Impulse> get_diffuse_impulses() const;
    aligned::vector<Impulse> get_image_source_impulses(
            bool remove_direct) const;

    map_type image_source;
    aligned::vector<aligned::vector<Impulse>> diffuse;
    glm::vec3 receiver;
    glm::vec3 source;
};

}  // namespace raytracer
