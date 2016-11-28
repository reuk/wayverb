#pragma once

#include "../../UtilityComponents/AngularLookAndFeel.h"
#include "combined/model/waveguide.h"

#include "../JuceLibraryCode/JuceHeader.h"

namespace left_bar {
namespace waveguide {

class master final : public TabbedComponent {
public:
    using model_t = wayverb::combined::model::waveguide;

    master(model_t& model);

    void currentTabChanged(int new_index, const String& name) override;

private:
    model_t& model_;
    model_t::scoped_connection connection_;
    bool initializing_ = true;
};

}  // namespace waveguide
}  // namespace left_bar
