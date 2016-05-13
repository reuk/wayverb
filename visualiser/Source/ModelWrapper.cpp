#include "ModelWrapper.hpp"

namespace model {

ChangeConnector::ChangeConnector(ChangeBroadcaster* cb, ChangeListener* cl)
        : cb(cb)
        , cl(cl) {
    if (cb && cl) {
        cb->addChangeListener(cl);
    }
}
ChangeConnector::~ChangeConnector() noexcept {
    if (cb && cl) {
        cb->removeChangeListener(cl);
    }
}

//----------------------------------------------------------------------------//

void Model::changeListenerCallback(ChangeBroadcaster* cb) {
    notify();
}

void Model::notify() {
    sendSynchronousChangeMessage();
}

//----------------------------------------------------------------------------//

ModelMember::ModelMember(ModelMember* parent)
        : parent(parent) {
}

ModelMember* ModelMember::get_parent() const {
    return parent;
}

//----------------------------------------------------------------------------//

Combined::Combined(ModelMember* parent, const config::Combined& rhs)
        : ModelMember(parent)
        , data(rhs) {
}

const config::Combined& Combined::get_data() const {
    return data;
}

//----------------------------------------------------------------------------//

}  // namespace model