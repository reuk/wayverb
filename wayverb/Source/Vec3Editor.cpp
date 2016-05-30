#include "Vec3Editor.hpp"

Vec3Editor::Vec3Editor(model::ValueWrapper<glm::vec3>& value,
                       const glm::vec3& min,
                       const glm::vec3& max)
        : value(value) {
    property_panel.addProperties(
        {new NumberProperty<float>("x", value.x, min.x, max.x),
         new NumberProperty<float>("y", value.y, min.y, max.y),
         new NumberProperty<float>("z", value.z, min.z, max.z)});

    property_panel.setOpaque(false);

    addAndMakeVisible(property_panel);
}

void Vec3Editor::resized() {
    property_panel.setBounds(getLocalBounds());
}

Vec3Property::Vec3Property(const String& name,
                           model::ValueWrapper<glm::vec3>& value,
                           const glm::vec3& min,
                           const glm::vec3& max)
        : PropertyComponent(name, 79)
        , editor(value, min, max) {
    addAndMakeVisible(editor);
}
void Vec3Property::refresh() {
}