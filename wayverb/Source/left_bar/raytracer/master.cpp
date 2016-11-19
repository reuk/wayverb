#include "master.h"

#include "../../UtilityComponents/connector.h"
#include "../../generic_combo_box_property.h"
#include "../../generic_slider_property.h"

#include "utilities/string_builder.h"

#include <iomanip>

namespace left_bar {
namespace raytracer {

ray_number_property::ray_number_property(model_t& model)
        : generic_combo_box_property{
                  model,
                  "rays",
                  {model_t::ray_number::r1e3,
                   model_t::ray_number::r1e4,
                   model_t::ray_number::r1e5,
                   model_t::ray_number::r1e6},
                  [](auto e) { return util::build_string(get_ray_number_description(e) , "\t(" , std::setprecision(1), std::scientific, get_ray_number(e) , ')'); }} {
    update_from_model();
}

void ray_number_property::set_model(model_t& model, const value_t& e) {
    model.set_ray_number(e);
}

ray_number_property::value_t ray_number_property::get_model(
        const model_t& model) const {
    return model.get_ray_number();
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
