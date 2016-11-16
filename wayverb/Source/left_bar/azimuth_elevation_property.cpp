#include "azimuth_elevation_property.h"

#include "slider_property.h"

#include "core/az_el.h"
#include "utilities/range.h"

namespace left_bar {

namespace {
constexpr auto elevation_range = util::make_range(-M_PI / 2, M_PI / 2);
}//namespace

azimuth_elevation_editor::azimuth_elevation_editor() {
    auto az = std::make_unique<slider_property>("azimuth", -M_PI, M_PI);
    auto el =
            std::make_unique<slider_property>("elevation", elevation_range.get_min(), elevation_range.get_max());

    const auto callback = [this](auto&, auto) { on_change_(*this, get()); };

    az->connect_on_change(callback);
    el->connect_on_change(callback);

    addProperties({az_ = az.release(), el_ = el.release()});
}

azimuth_elevation_editor::on_change::connection
azimuth_elevation_editor::connect_on_change(on_change::callback_type callback) {
    return on_change_.connect(std::move(callback));
}

wayverb::core::az_el azimuth_elevation_editor::get() const {
    return wayverb::core::az_el(az_->get(), el_->get());
}

void azimuth_elevation_editor::set(wayverb::core::az_el az_el) {
    az_->set(az_el.azimuth);
    el_->set(az_el.elevation);
}

////////////////////////////////////////////////////////////////////////////////

azimuth_elevation_property::azimuth_elevation_property(const String& name)
        : PropertyComponent{name, 54}
{
    editor_.connect_on_change(
            [this](auto&, auto az_el) { on_change_(*this, az_el); });

    addAndMakeVisible(editor_);
}

azimuth_elevation_property::on_change::connection
azimuth_elevation_property::connect_on_change(
        on_change::callback_type callback) {
    return on_change_.connect(std::move(callback));
}

void azimuth_elevation_property::refresh() {}

wayverb::core::az_el azimuth_elevation_property::get() const {
    return editor_.get();
}

void azimuth_elevation_property::set(const wayverb::core::az_el& az_el) {
    editor_.set(az_el);
}

}  // namespace left_bar
