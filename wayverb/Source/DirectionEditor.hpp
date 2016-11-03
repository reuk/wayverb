#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "glm/glm.hpp"

/*
class DirectionEditor : public TabbedComponent,
                        public model::BroadcastListener {
public:
    DirectionEditor(model::ValueWrapper<model::Pointer>& pointer);

    void currentTabChanged(int new_tab, const String& name) override;

    void receive_broadcast(model::Broadcaster* b) override;

private:
    model::ValueWrapper<model::Pointer>& pointer;
    model::BroadcastConnector pointer_connector{&pointer.mode, this};
};

//----------------------------------------------------------------------------//

class DirectionProperty : public PropertyComponent {
public:
    DirectionProperty(model::ValueWrapper<model::Pointer>& pointer);

    void refresh() override;

private:
    DirectionEditor direction_editor;
};
*/
