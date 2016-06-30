#include "DirectionEditor.hpp"
#include "Vec3Editor.hpp"

#include "common/orientable.h"

class SphericalEditor : public Component {
public:
    SphericalEditor(model::ValueWrapper<Orientable::AzEl>& azel) {
        property_panel.addProperties(
                {new NumberProperty<float>(
                         "azimuth", azel.azimuth, -M_PI, M_PI),
                 new NumberProperty<float>("elevation",
                                           azel.elevation,
                                           -M_PI * 0.49,
                                           M_PI * 0.49)});
        property_panel.setOpaque(false);
        addAndMakeVisible(property_panel);
    }

    void resized() override {
        property_panel.setBounds(getLocalBounds());
    }

private:
    PropertyPanel property_panel;
};

class BooleanProperty : public PropertyComponent,
                        public model::BroadcastListener,
                        public Button::Listener {
public:
    BooleanProperty(const String& name,
                    model::ValueWrapper<bool>& value,
                    const String& button_text)
            : PropertyComponent(name)
            , value(value)
            , button(button_text) {
        addAndMakeVisible(button);
    }

    void refresh() override {
    }

    void receive_broadcast(model::Broadcaster* b) override {
        if (b == &value) {
            button.setToggleState(value.get(), dontSendNotification);
        }
    }

    void buttonClicked(Button* b) override {
        if (b == &button) {
            value.set(button.getToggleState());
        }
    }

private:
    model::ValueWrapper<bool>& value;
    model::BroadcastConnector value_connector{&value, this};

    ToggleButton button;
    model::Connector<ToggleButton> button_connector{&button, this};
};

class LookAtEditor : public Component {
public:
    LookAtEditor(model::ValueWrapper<glm::vec3>& look_at) {
        //  TODO (maybe) set min/max based on model dimensions
        property_panel.addProperties({new Vec3Property(
                "look at", look_at, glm::vec3(-1000), glm::vec3(1000))});
        addAndMakeVisible(property_panel);
    }

    void resized() override {
        property_panel.setBounds(getLocalBounds());
    }

private:
    PropertyPanel property_panel;
};

//----------------------------------------------------------------------------//

DirectionEditor::DirectionEditor(model::ValueWrapper<model::Pointer>& pointer)
        : TabbedComponent(TabbedButtonBar::Orientation::TabsAtTop)
        , pointer(pointer) {
    auto mode = pointer.mode.get();
    addTab("spherical",
           Colours::darkgrey,
           new SphericalEditor(pointer.spherical),
           true);
    addTab("look at",
           Colours::darkgrey,
           new LookAtEditor(pointer.look_at),
           true);
    pointer.mode.set(mode);
}

void DirectionEditor::currentTabChanged(int new_tab, const String& name) {
    switch (new_tab) {
        case 0:
            assert(name == "spherical");
            pointer.mode.set(model::Pointer::Mode::spherical);
            break;
        case 1:
            assert(name == "look at");
            pointer.mode.set(model::Pointer::Mode::look_at);
            break;
    }
}

void DirectionEditor::receive_broadcast(model::Broadcaster* b) {
    if (b == &pointer.mode) {
        setCurrentTabIndex(
                pointer.mode.get() == model::Pointer::Mode::spherical ? 0 : 1,
                false);
    }
}

//----------------------------------------------------------------------------//

DirectionProperty::DirectionProperty(
        model::ValueWrapper<model::Pointer>& pointer)
        : PropertyComponent("direction", 120)
        , direction_editor(pointer) {
    addAndMakeVisible(direction_editor);
}

void DirectionProperty::refresh() {
}