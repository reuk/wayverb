#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "glm/glm.hpp"

class DirectionEditor : public TabbedComponent {
public:
    DirectionEditor();

    void currentTabChanged(int new_tab, const String& name) override;
};

//----------------------------------------------------------------------------//

class DirectionProperty : public PropertyComponent {
public:
    DirectionProperty();

    void refresh() override;

private:
    DirectionEditor direction_editor;
};
