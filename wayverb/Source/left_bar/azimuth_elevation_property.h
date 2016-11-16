#pragma once

#include "utilities/event.h"

#include "../JuceLibraryCode/JuceHeader.h"

namespace wayverb {
namespace core {
struct az_el;
}
}

namespace left_bar {

class slider_property;

class azimuth_elevation_editor final : public PropertyPanel {
public:
    azimuth_elevation_editor();

    using on_change =
            util::event<azimuth_elevation_editor&, wayverb::core::az_el>;
    on_change::connection connect_on_change(on_change::callback_type callback);

    wayverb::core::az_el get() const;

    void set(wayverb::core::az_el az_el);

private:
    slider_property* az_;
    slider_property* el_;
    on_change on_change_;
};

////////////////////////////////////////////////////////////////////////////////

class azimuth_elevation_property final : public PropertyComponent {
public:
    azimuth_elevation_property(const String& name);

    using on_change =
            util::event<azimuth_elevation_property&, wayverb::core::az_el>;
    on_change::connection connect_on_change(on_change::callback_type callback);

    void refresh() override;

    wayverb::core::az_el get() const;
    void set(const wayverb::core::az_el& az_el);

private:
    azimuth_elevation_editor editor_;
    on_change on_change_;
};

}  // namespace left_bar
