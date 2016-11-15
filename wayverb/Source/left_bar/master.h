#pragma once

#include "bottom_panel.h"
#include "../HelpWindow.h"
#include "../UtilityComponents/async_work_queue.h"

namespace wayverb { namespace combined { namespace model { class app; } } } 

namespace left_bar {

class master : public Component, public SettableHelpPanelClient {
public:
    master(wayverb::combined::model::app& model);

    void resized() override;

private:
    wayverb::combined::model::app& model_;
    async_work_queue queue_;

    PropertyPanel property_panel_;
    bottom bottom_;
};

}  // namespace left_bar
