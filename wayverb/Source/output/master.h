#pragma once

#include "../UtilityComponents/modal_dialog.h"

#include "combined/model/output.h"

#include "../JuceLibraryCode/JuceHeader.h"

#include <experimental/optional>

namespace output {

////////////////////////////////////////////////////////////////////////////////

/// Fire a modal configurator.
/// If configuration succeeds, pass the result to the callback.
void get_output_options(wayverb::combined::model::output& model,
                        modal_callback);

}  // namespace output
