#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "glm/glm.hpp"

class DirectionEditor : public Component {
public:
    enum class Mode { cartesian, spherical, look_at };

    glm::vec3 get_pointing_vector() const;

private:
    TabbedComponent tabbed_component;
};
