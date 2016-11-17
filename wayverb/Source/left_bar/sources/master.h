#pragma once

#include "../editable_vector_list_box.h"
#include "../list_config_item.h"

#include "combined/model/source.h"

namespace left_bar {
namespace sources {

/// The heart of the sources component is a list-box.
/// This should have buttons for adding and removing items.
/// If an item is added, show its pop-out immediately, with the name field
/// selected for editing.
/// The pop-out should contain name and position editors.
/// Don't allow sources to be added or removed if any are currently selected/
/// dragged.

class master final : public Component {
public:
    master(wayverb::combined::model::sources& sources);

    void resized() override;

private:
    editable_vector_list_box<wayverb::combined::model::sources> list_box_;
};

}  // namespace sources
}  // namespace left_bar
