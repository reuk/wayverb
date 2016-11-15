#include "master.h"

#include "combined/model/app.h"

namespace left_bar {
namespace sources {

/// The heart of the sources component is a list-box.
/// This should have buttons for adding and removing items.
/// If an item is added, show its pop-out immediately, with the name field
/// selected for editing.
/// The pop-out should contain name and position editors.
/// Don't allow sources to be added or removed if any are currently selected/
/// dragged.

master::master(wayverb::combined::model::app& app)
        : model_{app}
        , list_box_{model_.project.persistent.sources(),
                    [](int row, bool selected) {
                        return std::make_unique<Component>();
                    }} {}

}  // namespace sources
}  // namespace left_bar
