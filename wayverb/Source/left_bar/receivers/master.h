#pragma once

#include "../editable_vector_list_box.h"
#include "../list_config_item.h"

#include "combined/model/app.h"

namespace left_bar {
namespace receivers {

/// The heart of the receivers component is a list-box.
/// This should have buttons for adding and removing items.
/// If an item is added, show its pop-out immediately, with the name field
/// selected for editing.
/// The pop-out should contain name and position editors.
/// Don't allow receivers to be added or removed if any are currently selected/
/// dragged.

class master final : public Component {
public:
    master(const wayverb::combined::model::app::capsule_presets_t& presets,
           wayverb::core::geo::box aabb,
           wayverb::combined::model::receivers& receivers);

    void resized() override;

private:
    editable_vector_list_box<wayverb::combined::model::receivers> list_box_;
};

}  // namespace receivers
}  // namespace left_bar