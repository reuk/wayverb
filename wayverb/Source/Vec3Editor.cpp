#include "Vec3Editor.h"

Vec3Editor::Vec3Editor() {
    property_panel.addProperties(
            {new NumberProperty<float>("x"),
             new NumberProperty<float>("y"),
             new NumberProperty<float>("z")});

    property_panel.setOpaque(false);

    addAndMakeVisible(property_panel);
}

void Vec3Editor::resized() {
    property_panel.setBounds(getLocalBounds());
}

Vec3Property::Vec3Property(const String& name)
        : PropertyComponent(name, 75) {
    addAndMakeVisible(editor);
}

void Vec3Property::refresh() {
}
