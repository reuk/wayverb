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

private:
    bool hovered_ = false;
    bool selected_ = false;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
