#pragma once

#include "../generic_property_component.h"

#include "combined/model/output.h"

#include "../JuceLibraryCode/JuceHeader.h"

#include <experimental/optional>

namespace output {

////////////////////////////////////////////////////////////////////////////////

using output_options_callback = std::function<void(int status)>;

/// Fire a modal configurator.
/// If configuration succeeds, pass the result to the callback.
void get_output_options(wayverb::combined::model::output& model,
                        output_options_callback);

}  // namespace output
