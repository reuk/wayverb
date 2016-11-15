#include "master.h"
#include "../list_config_item.h"

#include "combined/model/app.h"

namespace left_bar {
namespace sources {

////////////////////////////////////////////////////////////////////////////////

master::master(wayverb::combined::model::app& app)
        : model_{app}
        , list_box_{model_.project.persistent.sources(),
                    [this](int row, bool selected) {
                        auto ret = std::make_unique<list_config_item>(
                            [] {return std::unique_ptr<Component>();}
                        );
                        const auto &sources = model_.project.persistent.sources();
                        jassert(0 <= row && row < sources.size());
                        ret->set_label(sources[row].get_name());
                        ret->setInterceptsMouseClicks(false, true);
                        return ret;
                    }} {
    list_box_.setRowHeight(30);
    addAndMakeVisible(list_box_);
}

void master::resized() {
    list_box_.setBounds(getLocalBounds());
}

}  // namespace sources
}  // namespace left_bar
