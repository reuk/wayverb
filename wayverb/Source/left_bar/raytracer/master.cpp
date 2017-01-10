#include "master.h"

#include "utilities/string_builder.h"

#include <iomanip>

namespace left_bar {
namespace raytracer {

rays_required_property::rays_required_property(model_t& model)
        : text_display_property{model, "rays"} {
    this->update_from_model();
}

std::string rays_required_property::get_model(const model_t& model) const {
    return std::to_string(model.get().rays);
}

////////////////////////////////////////////////////////////////////////////////

quality_property::quality_property(model_t& model)
        : generic_slider_property{model, "quality", 1, 20, 1} {
    update_from_model();
}

void quality_property::set_model(model_t& model, const value_t& e) {
    model.set_quality(e);
}

quality_property::value_t quality_property::get_model(
        const model_t& model) const {
    return model.get_quality();
}

////////////////////////////////////////////////////////////////////////////////

img_src_order_property::img_src_order_property(model_t& model)
        : generic_slider_property{model, "img-src levels", 0, 10, 1} {
    update_from_model();
}

void img_src_order_property::set_model(model_t& model, const value_t& e) {
    model.set_max_img_src_order(e);
}

img_src_order_property::value_t img_src_order_property::get_model(
        const model_t& model) const {
    return model.get_max_img_src_order();
}

}  // namespace raytracer
}  // namespace left_bar
