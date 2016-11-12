#pragma once

#include "HelpWindow.h"
#include "ValueWrapperSlider.h"

#include "UtilityComponents/NumberEditor.h"

#include "glm/glm.hpp"

#include <array>
#include <iomanip>
#include <sstream>

template <typename T>
class NumberProperty : public PropertyComponent {
public:
    NumberProperty(const String& name)
            : PropertyComponent(name) {
        addAndMakeVisible(editor);
    }

    void refresh() override {}

private:
    NumberEditor<T> editor;
};

class Vec3Editor : public Component {
public:
    Vec3Editor();
    void resized() override;

private:
    PropertyPanel property_panel;
};

class Vec3Property : public PropertyComponent, public SettableHelpPanelClient {
public:
    Vec3Property(const String& name);
    void refresh() override;

private:
    Vec3Editor editor;
};
