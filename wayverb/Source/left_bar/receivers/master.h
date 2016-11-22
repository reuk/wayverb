#pragma once

#include "../editable_vector_list_box.h"
#include "../list_config_item.h"

#include "main_model.h"

namespace left_bar {
namespace receivers {

/// The heart of the receivers component is a list-box.
/// This should have buttons for adding and removing items.
/// The pop-out should contain name and position editors.
/// Don't allow receivers to be added or removed if any are currently selected/
/// dragged.

class master final : public Component {
public:
    master(const main_model::capsule_presets_t& presets,
           wayverb::core::geo::box aabb,
           wayverb::combined::model::receivers& receivers);

    void resized() override;

private:
    editable_vector_list_box<wayverb::combined::model::receivers> list_box_;
};

}  // namespace receivers
}  // namespace left_bar
