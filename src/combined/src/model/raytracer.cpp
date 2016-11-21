#include "combined/model/raytracer.h"

namespace wayverb {
namespace combined {
namespace model {

raytracer::raytracer(ray_number ray_number, size_t img_src_order)
        : ray_number_{ray_number}
        , img_src_order_{img_src_order} {}

void raytracer::swap(raytracer& other) noexcept {
    using std::swap;
    swap(ray_number_, other.ray_number_);
    swap(img_src_order_, other.img_src_order_);
}

raytracer& raytracer::operator=(raytracer other) {
    base_type::operator=(other);
    swap(other);
    notify();
    return *this;
}

void raytracer::set_ray_number(ray_number ray_number) {
    ray_number_ = ray_number;
    notify();
}

raytracer::ray_number raytracer::get_ray_number() const { return ray_number_; }

void raytracer::set_max_img_src_order(size_t i) {
    img_src_order_ = i;
    notify();
}

size_t raytracer::get_max_img_src_order() const { return img_src_order_; }

wayverb::raytracer::simulation_parameters raytracer::get() const {
    return {wayverb::combined::model::get_ray_number(ray_number_),
            img_src_order_};
}

bool operator==(const raytracer& a, const raytracer& b) {
    return a.get() == b.get();
}

bool operator!=(const raytracer& a, const raytracer& b) { return !(a == b); }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
