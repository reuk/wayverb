#include "DirectionEditor.h"
#include "Vec3Editor.h"

#include "UtilityComponents/connector.h"

#include "core/orientation.h"

class SphericalEditor : public Component {
public:
    SphericalEditor() {
        property_panel.addProperties({new NumberProperty<float>("azimuth"),
                                      new NumberProperty<float>("elevation")});
        property_panel.setOpaque(false);
        addAndMakeVisible(property_panel);
    }

    void resized() override { property_panel.setBounds(getLocalBounds()); }

private:
    PropertyPanel property_panel;
};

class BooleanProperty : public PropertyComponent, public Button::Listener {
public:
    BooleanProperty(const String& name, const String& button_text)
            : PropertyComponent(name)
            , button(button_text) {
        addAndMakeVisible(button);
    }

    void refresh() override {}

    void buttonClicked(Button* b) override {
        if (b == &button) {
            // value.set(button.getToggleState());
        }
    }

private:
    ToggleButton button;
    model::Connector<ToggleButton> button_connector{&button, this};
};

class LookAtEditor : public Component {
public:
    LookAtEditor() {
        property_panel.addProperties({new Vec3Property("look at")});
        addAndMakeVisible(property_panel);
    }

    void resized() override { property_panel.setBounds(getLocalBounds()); }

private:
    PropertyPanel property_panel;
};

//----------------------------------------------------------------------------//

DirectionEditor::DirectionEditor()
        : TabbedComponent(TabbedButtonBar::Orientation::TabsAtTop) {
    addTab("spherical", Colours::darkgrey, new SphericalEditor(), true);
    addTab("look at", Colours::darkgrey, new LookAtEditor(), true);
}

void DirectionEditor::currentTabChanged(int new_tab, const String& name) {
    switch (new_tab) {
        case 0: assert(name == "spherical"); break;
        case 1: assert(name == "look at"); break;
    }
}

//----------------------------------------------------------------------------//

DirectionProperty::DirectionProperty()
        : PropertyComponent("direction", 120) {
    addAndMakeVisible(direction_editor);
}

void DirectionProperty::refresh() {}
