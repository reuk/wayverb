#pragma once

#include "utilities/string_builder.h"

#include "../JuceLibraryCode/JuceHeader.h"

#include <string>

template <typename Callback>
void try_and_explain(const Callback& callback,
                     const std::string& action_name,
                     const std::string& additional_info = "") {
    try {
        callback();
    } catch (const std::exception& e) {
        AlertWindow::showMessageBoxAsync(
                AlertWindow::AlertIconType::WarningIcon,
                "error",
                util::build_string("Encountered an error ",
                                   action_name,
                                   ":\n",
                                   e.what(),
                                   '\n',
                                   additional_info));
    }
}
