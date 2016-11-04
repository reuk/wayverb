#pragma once

#include "core/cl/scene_structs.h"

#include "glm/glm.hpp"

namespace wayverb {
namespace core {
namespace attenuator {

class hrtf final {
public:
    enum class channel { left, right };

    hrtf() = default;
    hrtf(const glm::vec3& pointing,
         const glm::vec3& up,
         channel channel,
         float radius = 0.1);

    glm::vec3 get_pointing() const;
    glm::vec3 get_up() const;
    channel get_channel() const;
    float get_radius() const;

    void set_pointing(const glm::vec3& pointing);
    void set_up(const glm::vec3& up);
    void set_channel(channel channel);
    void set_radius(float radius);

    template <typename Archive>
    void serialize(Archive&);
    
private:
    glm::vec3 pointing_;
    glm::vec3 up_;
    channel channel_;
    float radius_;
};

glm::vec3 transform(const glm::vec3& pointing,
                    const glm::vec3& up,
                    const glm::vec3& d);

template <typename T, size_t... Ix>
constexpr auto to_cl_float_vector(const std::array<T, sizeof...(Ix)>& x,
                                  std::index_sequence<Ix...>) {
    using return_type = ::detail::cl_vector_constructor_t<float, sizeof...(Ix)>;
    return return_type{{static_cast<float>(std::get<Ix>(x))...}};
}

template <typename T, size_t I>
constexpr auto to_cl_float_vector(const std::array<T, I>& x) {
    return to_cl_float_vector(x, std::make_index_sequence<I>{});
}

bands_type attenuation(const hrtf& hrtf, const glm::vec3& incident);

glm::vec3 get_ear_position(const hrtf& hrtf, const glm::vec3& base_position);

}  // namespace attenuator
}  // namespace core
}  // namespace wayverb
