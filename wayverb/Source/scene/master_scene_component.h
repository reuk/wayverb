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

class master_scene_component final : public Component,
                                     public SettableHelpPanelClient {
public:
    master_scene_component(wayverb::combined::model::app& app);
    ~master_scene_component() noexcept;

    void resized() override;

private:
    //  I have a feeling the scene view stuff is going to change a whole bunch,
    //  so a complier firewall here should help speed up builds. Hopefully.
    class impl;
    std::unique_ptr<impl> pimpl_;
};
