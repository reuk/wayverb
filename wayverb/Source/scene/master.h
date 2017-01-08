#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include <memory>

class main_model;

namespace scene {

class master final : public Component, public SettableTooltipClient {
public:
    master(main_model& model);
    ~master() noexcept;

    void resized() override;

private:
    //  I have a feeling the scene view stuff is going to change a whole bunch,
    //  so a complier firewall here should help speed up builds. Hopefully.
    class impl;
    std::unique_ptr<impl> pimpl_;
};

}  // namespace scene
