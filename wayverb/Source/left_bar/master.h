#pragma once

#include "../HelpWindow.h"

#include "bottom_panel.h"
#include "engine_message_queue.h"

namespace left_bar {

class master : public Component, public SettableHelpPanelClient {
public:
    master(wayverb::combined::model::app& model, engine_message_queue& queue);

    void resized() override;

private:
    wayverb::combined::model::app& model_;

    engine_message_queue::begun::scoped_connection begun_connection_;
    engine_message_queue::engine_state_changed::scoped_connection
            engine_state_connection_;
    engine_message_queue::finished::scoped_connection finished_connection_;

    PropertyPanel property_panel_;
    bottom bottom_;
};

}  // namespace left_bar
