#include "vec3_property.h"

#include "slider_property.h"

namespace left_bar {

vec3_editor::vec3_editor(const util::range<glm::vec3>& range) {
    auto x = std::make_unique<slider_property>(
            "x", range.get_min().x, range.get_max().x, 0.001, " m");
    auto y = std::make_unique<slider_property>(
            "y", range.get_min().y, range.get_max().y, 0.001, " m");
    auto z = std::make_unique<slider_property>(
            "z", range.get_min().z, range.get_max().z, 0.001, " m");

    const auto callback = [this](auto&, auto) { on_change_(*this, get()); };

    x->connect_on_change(callback);
    y->connect_on_change(callback);
    z->connect_on_change(callback);

    addProperties({x_ = x.release(), y_ = y.release(), z_ = z.release()});
}

vec3_editor::on_change::connection vec3_editor::connect_on_change(
        on_change::callback_type callback) {
    return on_change_.connect(std::move(callback));
}

glm::vec3 vec3_editor::get() const {
    return glm::vec3(x_->get(), y_->get(), z_->get());
}

void vec3_editor::set(const glm::vec3& v) {
    x_->set(v.x);
    y_->set(v.y);
    z_->set(v.z);
}

////////////////////////////////////////////////////////////////////////////////

vec3_property::vec3_property(const String& name,
                             const util::range<glm::vec3>& range)
        : PropertyComponent{name, 79}
        , editor_{range} {
    editor_.connect_on_change(
            [this](auto&, auto vec) { on_change_(*this, vec); });

    addAndMakeVisible(editor_);
}

vec3_property::on_change::connection vec3_property::connect_on_change(
        on_change::callback_type callback) {
    return on_change_.connect(std::move(callback));
}

void vec3_property::refresh() {}

glm::vec3 vec3_property::get() const { return editor_.get(); }
void vec3_property::set(const glm::vec3& v) { editor_.set(v); }

}  // namespace left_bar
