#pragma once

#include "FullModel.hpp"

#include "../JuceLibraryCode/JuceHeader.h"

#include "glm/glm.hpp"

class DirectionEditor : public TabbedComponent,
                        public model::BroadcastListener {
public:
    enum class Mode { cartesian, spherical, look_at };

    DirectionEditor(model::ValueWrapper<glm::vec3>& pointing,
                    model::ValueWrapper<glm::vec3>& target);

    void receive_broadcast(model::Broadcaster* b) override;

private:
    model::ValueWrapper<glm::vec3>& pointing;
    model::BroadcastConnector pointing_connector{&pointing, this};

    model::ValueWrapper<glm::vec3>& target;
    model::BroadcastConnector target_connector{&target, this};
};

//----------------------------------------------------------------------------//

class DirectionProperty : public PropertyComponent {
public:
    DirectionProperty(model::ValueWrapper<glm::vec3>& pointing,
                      model::ValueWrapper<glm::vec3>& target);

    void refresh() override;

private:
    DirectionEditor direction_editor;
};