#include "DirectionEditor.hpp"
#include "Orientable.hpp"
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
            {new NumberProperty<float>("azimuth", azimuth_wrapper, -180, 180),
             new NumberProperty<float>(
                 "elevation", elevation_wrapper, -89, 89)});
        property_panel.setOpaque(false);
        addAndMakeVisible(property_panel);

        auto n = glm::normalize(pointing.get());
        azimuth_wrapper.set(glm::degrees(Orientable::compute_azimuth(n)));
        elevation_wrapper.set(glm::degrees(Orientable::compute_elevation(n)));
    }

    void resized() override {
        property_panel.setBounds(getLocalBounds());
    }

    void receive_broadcast(model::Broadcaster* b) override {
        if (b == &azimuth_wrapper || b == &elevation_wrapper) {
            pointing.set(Orientable::compute_pointing(
                Orientable::AzEl{glm::radians(azimuth_wrapper.get()),
                                 glm::radians(elevation_wrapper.get())}));
        }
    }

private:
    model::ValueWrapper<glm::vec3>& pointing;

    float azimuth{0};
    model::ValueWrapper<float> azimuth_wrapper{nullptr, azimuth};
    model::BroadcastConnector azimuth_connector{&azimuth_wrapper, this};
    float elevation{0};
    model::ValueWrapper<float> elevation_wrapper{nullptr, elevation};
    model::BroadcastConnector elevation_connector{&elevation_wrapper, this};

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
            button.setToggleState(value, dontSendNotification);
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

class LookAtEditor : public Component, public model::BroadcastListener {
public:
    LookAtEditor(model::ValueWrapper<glm::vec3>& pointing,
                 model::ValueWrapper<glm::vec3>& position)
            : pointing(pointing)
            , position(position) {
        property_panel.addProperties({new Vec3Property(
            "target", target_wrapper, glm::vec3(-1000), glm::vec3(1000))});
        addAndMakeVisible(property_panel);

        target_wrapper.set(position.get() + pointing.get());
    }

    void resized() override {
        property_panel.setBounds(getLocalBounds());
    }

    void receive_broadcast(model::Broadcaster* b) override {
        if (b == &target_wrapper) {
            pointing.set(glm::normalize(target_wrapper.get() - position.get()));
        }
    }

private:
    model::ValueWrapper<glm::vec3>& pointing;
    model::ValueWrapper<glm::vec3>& position;

    glm::vec3 target{0, 0, 0};
    model::ValueWrapper<glm::vec3> target_wrapper{nullptr, target};
    model::BroadcastConnector target_connector{&target_wrapper, this};

    PropertyPanel property_panel;
};

//----------------------------------------------------------------------------//

DirectionEditor::DirectionEditor(model::ValueWrapper<glm::vec3>& pointing,
                                 model::ValueWrapper<glm::vec3>& position)
        : pointing(pointing)
        , position(position)
        , tabs(TabbedButtonBar::TabsAtTop) {
    tabs.addTab("cartesian", Colours::darkgrey, -1);
    tabs.addTab("spherical", Colours::darkgrey, -1);
    tabs.addTab("look at", Colours::darkgrey, -1);

    addAndMakeVisible(tabs);
}

void DirectionEditor::resized() {
    auto bounds = getLocalBounds();
    auto tab_height = 30;
    tabs.setBounds(bounds.removeFromTop(tab_height));
    bounds.reduce(2, 2);
    if (content) {
        content->setBounds(bounds);
    }
}

void DirectionEditor::paint(Graphics& g) {
    auto content = getLocalBounds();
    BorderSize<int> outline(1);

    content.removeFromTop(tabs.getHeight() - 1);

    g.reduceClipRegion(content);
    g.fillAll(tabs.getTabBackgroundColour(tabs.getCurrentTabIndex()));

    RectangleList<int> rl(content);
    rl.subtract(outline.subtractedFrom(content));

    g.reduceClipRegion(rl);
    g.fillAll(tabs.findColour(TabbedButtonBar::ColourIds::tabOutlineColourId));
}

void DirectionEditor::changeListenerCallback(ChangeBroadcaster* cb) {
    if (cb == &tabs) {
        switch (tabs.getCurrentTabIndex()) {
            case 0:  // cartesian
                content = std::make_unique<CartesianEditor>(
                    pointing, glm::vec3(-1), glm::vec3(1));
                break;
            case 1:  // spherical
                content = std::make_unique<SphericalEditor>(pointing);
                break;
            case 2:  // look at
                content = std::make_unique<LookAtEditor>(pointing, position);
                break;
        }
        addAndMakeVisible(*content);
        resized();
    }
}

//----------------------------------------------------------------------------//

DirectionProperty::DirectionProperty(model::ValueWrapper<glm::vec3>& pointing,
                                     model::ValueWrapper<glm::vec3>& position)
        : PropertyComponent("direction", 200)
        , direction_editor(pointing, position) {
    addAndMakeVisible(direction_editor);
}

void DirectionProperty::refresh() {
}