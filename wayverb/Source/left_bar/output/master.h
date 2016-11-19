#pragma once

#include "../generic_property_component.h"

#include "combined/model/output.h"

#include "../JuceLibraryCode/JuceHeader.h"

namespace left_bar {
namespace output {

class master final : public PropertyPanel, public ComboBox::Listener {
public:
    using model_t = wayverb::combined::model::output;

    master(model_t& model);

    void comboBoxChanged(ComboBox* cb) override;

private:
    model_t& model_;
    model_t::scoped_connection connection_;

    //  Non-owning, don't worry.
    generic_property_component<ComboBox>* bit_depth_;
    generic_property_component<ComboBox>* sample_rate_;
    generic_property_component<ComboBox>* format_;
};

}  // namespace output
}  // left_bar
