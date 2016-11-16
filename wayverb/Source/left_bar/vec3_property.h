#pragma once

#include "utilities/event.h"
#include "utilities/range.h"

#include "glm/glm.hpp"

#include "../JuceLibraryCode/JuceHeader.h"

namespace left_bar {

class slider_property;

class vec3_editor final : public PropertyPanel {
public:
    vec3_editor(const util::range<glm::vec3>& range);

    using on_change = util::event<vec3_editor&, glm::vec3>;
    on_change::connection connect_on_change(on_change::callback_type callback);

    glm::vec3 get() const;

    void set(const glm::vec3& v);

private:
    slider_property* x_;
    slider_property* y_;
    slider_property* z_;
    on_change on_change_;
};

////////////////////////////////////////////////////////////////////////////////

class vec3_property final : public PropertyComponent {
public:
    vec3_property(const String& name, const util::range<glm::vec3>& range);

    using on_change = util::event<vec3_property&, glm::vec3>;
    on_change::connection connect_on_change(on_change::callback_type callback);

    void refresh() override;

    glm::vec3 get() const;
    void set(const glm::vec3& v);

private:
    vec3_editor editor_;
    on_change on_change_;
};

}  // namespace left_bar
