#pragma once

#include "combined/model/member.h"

namespace wayverb {
namespace combined {
namespace model {

class hover_state final : public basic_member<hover_state> {
public:
    void set_hovered(bool hovered);
    bool get_hovered() const;

    void set_selected(bool selected);
    bool get_selected() const;

    NOTIFYING_COPY_ASSIGN_DECLARATION(hover_state)
private:
    void swap(hover_state& other) noexcept {
        using std::swap;
        swap(hovered_, other.hovered_);
        swap(selected_, other.selected_);
    }

    bool hovered_ = false;
    bool selected_ = false;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
