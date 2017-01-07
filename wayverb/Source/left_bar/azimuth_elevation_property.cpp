#include "azimuth_elevation_property.h"

#include "slider_property.h"

#include "core/az_el.h"
#include "utilities/range.h"

namespace left_bar {

namespace {
constexpr auto elevation_range = util::make_range(-M_PI / 2, M_PI / 2);

constexpr auto radians(double t) {
    return t * M_PI / 180;
}

constexpr auto degrees(double t) {
    return t * 180 / M_PI;
}

}  // namespace

azimuth_elevation_editor::azimuth_elevation_editor() {
    auto az = std::make_unique<slider_property>("azimuth", degrees(-M_PI), degrees(M_PI), 1, " deg");
    auto el = std::make_unique<slider_property>(
            "elevation", degrees(elevation_range.get_min()), degrees(elevation_range.get_max()), 1, " deg");

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
    return wayverb::core::az_el(radians(az_->get()), radians(el_->get()));
}

void azimuth_elevation_editor::set(wayverb::core::az_el az_el) {
    az_->set(degrees(az_el.azimuth));
    el_->set(degrees(az_el.elevation));
}

////////////////////////////////////////////////////////////////////////////////

azimuth_elevation_property::azimuth_elevation_property(const String& name)
        : PropertyComponent{name, 54} {
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
