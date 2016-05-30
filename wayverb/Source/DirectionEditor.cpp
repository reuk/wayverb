#include "DirectionEditor.hpp"

#include "Vec3Editor.hpp"

class CartesianEditor : public Vec3Editor {
public:
    using Vec3Editor::Vec3Editor;
};

class SphericalEditor : public Component, public model::BroadcastListener {
public:
    SphericalEditor(model::ValueWrapper<glm::vec3>& pointing)
            : pointing(pointing) {
        property_panel.addProperties(
            {new NumberProperty<float>("azimuth", azimuth_wrapper, -M_PI, M_PI),
             new NumberProperty<float>(
                 "elevation", elevation_wrapper, -M_PI / 2, M_PI / 2)});
        property_panel.setOpaque(false);
        addAndMakeVisible(property_panel);

        pointing.notify();
    }

    void resized() override {
        property_panel.setBounds(getLocalBounds());
    }

    void receive_broadcast(model::Broadcaster* b) override {
        if (b == &azimuth_wrapper) {
            //  TODO update pointing properly
        } else if (b == &elevation_wrapper) {
            //  TODO update pointing properly
        } else if (b == &pointing) {
            //  TODO update azimuth and elevation properly
        }
    }

private:
    model::ValueWrapper<glm::vec3>& pointing;
    model::BroadcastConnector pointing_connector{&pointing, this};

    float azimuth;
    float elevation;
    model::ValueWrapper<float> azimuth_wrapper{nullptr, azimuth};
    model::BroadcastConnector azimuth_connector{&azimuth_wrapper, this};
    model::ValueWrapper<float> elevation_wrapper{nullptr, elevation};
    model::BroadcastConnector elevation_connector{&elevation_wrapper, this};

    PropertyPanel property_panel;
};

class LookAtEditor : public Component {
public:
};

DirectionEditor::DirectionEditor(model::ValueWrapper<glm::vec3>& pointing,
                                 model::ValueWrapper<glm::vec3>& target)
        : TabbedComponent(TabbedButtonBar::Orientation::TabsAtTop)
        , pointing(pointing)
        , target(target) {
    addTab("cartesian",
           Colours::darkgrey,
           new CartesianEditor(pointing, glm::vec3(-1), glm::vec3(1)),
           true);
    addTab("spherical", Colours::darkgrey, new SphericalEditor(pointing), true);
    addTab("look at", Colours::darkgrey, new LookAtEditor, true);
}

void DirectionEditor::receive_broadcast(model::Broadcaster* b) {
    if (b == &pointing) {
    } else if (b == &target) {
        if (getCurrentTabIndex() == 2) {  // if currently in look-at mode
            //  if locked, do the thing
        }
    }
}

//----------------------------------------------------------------------------//

DirectionProperty::DirectionProperty(model::ValueWrapper<glm::vec3>& pointing,
                                     model::ValueWrapper<glm::vec3>& target)
        : PropertyComponent("direction", 200)
        , direction_editor(pointing, target) {
    addAndMakeVisible(direction_editor);
}

void DirectionProperty::refresh() {
}