#include "ModelWrapper.hpp"

namespace model {

void ModelMember::changeListenerCallback(ChangeBroadcaster* cb) {
    notify();
}

void ModelMember::notify() {
    sendSynchronousChangeMessage();
}

ModelMember::ModelMember(ModelMember* owner)
        : owner(owner) {
}

ModelMember* ModelMember::get_owner() const {
    return owner;
}

}  // namespace model