#include "combined/model/raytracer.h"

namespace wayverb {
namespace combined {
namespace model {

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

}  // namespace model
}  // namespace combined
}  // namespace wayverb
