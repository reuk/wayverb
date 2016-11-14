#pragma once

#include "core/cl/scene_structs.h"
#include "core/orientation.h"

#include "glm/glm.hpp"

namespace wayverb {
namespace core {
namespace attenuator {

class hrtf final {
public:
    using orientation_t = class orientation;

    enum class channel { left, right };

    explicit hrtf(const orientation_t& o = orientation_t(),
                  channel channel = channel::left,
                  float radius = 0.1f);

    void set_channel(channel channel);
    channel get_channel() const;

    void set_radius(float radius);
    float get_radius() const;

    template <typename Archive>
    void serialize(Archive&);

    orientation_t orientation;

private:
    channel channel_;
    float radius_;
};

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
