#include "combined/model/hover.h"

namespace wayverb {
namespace combined {
namespace model {

void hover_state::swap(hover_state& other) noexcept {
    using std::swap;
    swap(hovered_, other.hovered_);
    swap(selected_, other.selected_);
}

hover_state& hover_state::operator=(hover_state other) {
    base_type::operator=(other);
    swap(other);
    notify();
    return *this;
}

void hover_state::set_hovered(bool hovered) {
    const auto do_notify = hovered != hovered_;
    hovered_ = hovered;
    if (do_notify) {
        notify();
    }
}

bool hover_state::get_hovered() const { return hovered_; }

void hover_state::set_selected(bool selected) {
    const auto do_notify = selected != selected_;
    selected_ = selected;
    if (do_notify) {
        notify();
    }
}

bool hover_state::get_selected() const { return selected_; }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
