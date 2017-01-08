#pragma once

#include "bottom_panel.h"
#include "main_model.h"

namespace left_bar {

class master : public Component {
public:
    master(main_model& model);

    void resized() override;

private:
    main_model& model_;

    main_model::begun::scoped_connection begun_connection_;
    main_model::engine_state_changed::scoped_connection engine_state_connection_;
    main_model::finished::scoped_connection finished_connection_;

    PropertyPanel property_panel_;
    bottom bottom_;
};

}  // namespace left_bar
