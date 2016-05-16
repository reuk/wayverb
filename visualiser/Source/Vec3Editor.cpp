#include "Vec3Editor.hpp"

Vec3fEditor::Vec3fEditor(model::ValueWrapper<Vec3f>& value)
        : value(value) {
    property_panel.addProperties({new NumberProperty<float>("x", value.x),
                                  new NumberProperty<float>("y", value.y),
                                  new NumberProperty<float>("z", value.z)});

    property_panel.setOpaque(false);

    addAndMakeVisible(property_panel);
}

void Vec3fEditor::resized() {
    property_panel.setBounds(getLocalBounds());
}

Vec3fProperty::Vec3fProperty(const String& name,
                             model::ValueWrapper<Vec3f>& value)
        : PropertyComponent(name, 79)
        , editor(value) {
    addAndMakeVisible(editor);
}
void Vec3fProperty::refresh() {
}