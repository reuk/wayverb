#pragma once

#include "../../editable_vector_list_box.h"
#include "../../list_config_item.h"

#include "combined/model/capsule.h"
#include "combined/model/min_size_vector.h"

namespace left_bar {
namespace receivers {
namespace capsules {

class master final : public Component {
public:
    master(wayverb::combined::model::
                   min_size_vector<wayverb::combined::model::capsule, 1>& app);

    void resized() override;

private:
    editable_vector_list_box<wayverb::combined::model::min_size_vector<
            wayverb::combined::model::capsule,
            1>>
            list_box_;
};

}  // namespace capsules
}  // namespace receivers
}  // namespace left_bar
