#pragma once

#include "FullModel.hpp"

#include "../JuceLibraryCode/JuceHeader.h"

#include "glm/glm.hpp"

class DirectionEditor : public Component, public ChangeListener {
public:
    enum class Mode { cartesian, spherical, look_at };

    DirectionEditor(model::ValueWrapper<glm::vec3>& pointing,
                    model::ValueWrapper<glm::vec3>& position);

    void resized() override;
    void paint(Graphics& g) override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

private:
    model::ValueWrapper<glm::vec3>& pointing;
    model::ValueWrapper<glm::vec3>& position;

    //  Really this should be a TabbedComponent, but I want to reconstruct the
    //  shown panel every time the tab changes, which TabbedComponents don't do.
    TabbedButtonBar tabs;
    model::Connector<ChangeBroadcaster, ChangeListener> tabs_connector{&tabs,
                                                                       this};

    std::unique_ptr<Component> content;
};

//----------------------------------------------------------------------------//

class DirectionProperty : public PropertyComponent {
public:
    DirectionProperty(model::ValueWrapper<glm::vec3>& pointing,
                      model::ValueWrapper<glm::vec3>& position);

    void refresh() override;

private:
    DirectionEditor direction_editor;
};