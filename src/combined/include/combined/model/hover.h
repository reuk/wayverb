#pragma once

#include "combined/model/member.h"

namespace wayverb {
namespace combined {
namespace model {

class hover_state final : public basic_member<hover_state> {
public:
    hover_state& operator=(hover_state other);

    void set_hovered(bool hovered);
    bool get_hovered() const;

    void set_selected(bool selected);
    bool get_selected() const;

private:
    void swap(hover_state& other) noexcept;

    bool hovered_ = false;
    bool selected_ = false;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
