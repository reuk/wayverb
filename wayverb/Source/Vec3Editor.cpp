#include "Vec3Editor.hpp"

Vec3fEditor::Vec3fEditor(model::ValueWrapper<Vec3f>& value,
                         const Vec3f& min,
                         const Vec3f& max)
        : value(value) {
    property_panel.addProperties(
        {new NumberProperty<float>("x", value.x, min.x, max.x),
         new NumberProperty<float>("y", value.y, min.y, max.y),
         new NumberProperty<float>("z", value.z, min.z, max.z)});

    property_panel.setOpaque(false);

    addAndMakeVisible(property_panel);
}

void Vec3fEditor::resized() {
    property_panel.setBounds(getLocalBounds());
}

Vec3fProperty::Vec3fProperty(const String& name,
                             model::ValueWrapper<Vec3f>& value,
                             const Vec3f& min,
                             const Vec3f& max)
        : PropertyComponent(name, 79)
        , editor(value, min, max) {
    addAndMakeVisible(editor);
}
void Vec3fProperty::refresh() {
}