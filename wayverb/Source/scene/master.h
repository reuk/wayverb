#pragma once

#include "HelpWindow.h"

#include <memory>

namespace wayverb {
namespace combined {
namespace model {
class app;
}
}
}

class engine_message_queue;

namespace scene {

class master final : public Component, public SettableHelpPanelClient {
public:
    master(wayverb::combined::model::app& app, engine_message_queue& queue);
    ~master() noexcept;

    void resized() override;

private:
    //  I have a feeling the scene view stuff is going to change a whole bunch,
    //  so a complier firewall here should help speed up builds. Hopefully.
    class impl;
    std::unique_ptr<impl> pimpl_;
};

}  // namespace scene
