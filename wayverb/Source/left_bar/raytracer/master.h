#pragma once

#include "../../generic_combo_box_property.h"
#include "../../generic_slider_property.h"

#include "combined/model/raytracer.h"

namespace left_bar {
namespace raytracer {

class rays_required_property final
        : public text_display_property<wayverb::combined::model::raytracer> {
public:
    rays_required_property(model_t& model);

private:
    std::string get_model(const model_t& model) const override;
};

////////////////////////////////////////////////////////////////////////////////

class quality_property final
        : public generic_slider_property<wayverb::combined::model::raytracer> {
public:
    quality_property(model_t& model);

private:
    void set_model(model_t& model, const value_t& e) override;
    value_t get_model(const model_t& model) const override;
};

////////////////////////////////////////////////////////////////////////////////

class img_src_order_property final
        : public generic_slider_property<wayverb::combined::model::raytracer> {
public:
    img_src_order_property(model_t& model);

private:
    void set_model(model_t& model, const value_t& e) override;
    value_t get_model(const model_t& model) const override;
};

}  // namespace raytracer
}  // namespace left_bar
