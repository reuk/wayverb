#pragma once

#include "../../editable_vector_list_box.h"
#include "../../list_config_item.h"

#include "combined/model/vector.h"
#include "combined/model/capsule.h"

namespace left_bar {
namespace receivers {
namespace capsules {

class capsule_config_item final
        : public list_config_item<wayverb::combined::model::capsule> {
    std::unique_ptr<Component> get_callout_component(
            wayverb::combined::model::capsule& model) override;
};

class master final : public Component {
public:
    master(wayverb::combined::model::vector<wayverb::combined::model::capsule,
                                            1>& app);

    void resized() override;

private:
    editable_vector_list_box<
            wayverb::combined::model::vector<wayverb::combined::model::capsule,
                                             1>,
            capsule_config_item>
            list_box_;
};

}  // namespace capsules
}  // namespace receivers
}  // namespace left_bar
