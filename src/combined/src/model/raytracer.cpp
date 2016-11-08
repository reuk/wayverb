#include "combined/model/raytracer.h"

namespace wayverb {
namespace combined {
namespace model {

void raytracer::set_rays(size_t rays) {
    data_.rays = rays;
    notify();
}

void raytracer::set_max_img_src_order(size_t max) {
    data_.maximum_image_source_order = max;
    notify();
}

wayverb::raytracer::simulation_parameters raytracer::get() const {
    return data_;
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
