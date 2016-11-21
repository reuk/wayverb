#pragma once

#include "combined/model/member.h"

#include "raytracer/simulation_parameters.h"

namespace wayverb {
namespace combined {
namespace model {

class raytracer final : public basic_member<raytracer> {
public:
    enum class ray_number {
        r1e3 = 1000,
        r1e4 = 10000,
        r1e5 = 100000,
        r1e6 = 1000000
    };

    explicit raytracer(ray_number ray_number = ray_number::r1e4,
                       size_t img_src_order = 4);

    raytracer& operator=(raytracer other);

    void set_ray_number(ray_number rays);
    ray_number get_ray_number() const;

    void set_max_img_src_order(size_t max);
    size_t get_max_img_src_order() const;

    wayverb::raytracer::simulation_parameters get() const;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(ray_number_, img_src_order_);
    }

private:
    void swap(raytracer& other) noexcept;

    ray_number ray_number_;
    size_t img_src_order_;
};

bool operator==(const raytracer& a, const raytracer& b);
bool operator!=(const raytracer& a, const raytracer& b);

constexpr size_t get_ray_number(raytracer::ray_number r) {
    return static_cast<size_t>(r);
}

constexpr auto get_ray_number_description(raytracer::ray_number r) {
    switch (r) {
        case raytracer::ray_number::r1e3: return "few";
        case raytracer::ray_number::r1e4: return "some";
        case raytracer::ray_number::r1e5: return "lots";
        case raytracer::ray_number::r1e6: return "loads";
    }
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
