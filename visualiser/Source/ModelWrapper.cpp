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

void Vec3fWrapper::set_value(const Vec3f& u, bool do_notify) {
    x.set_value(u.x);
    y.set_value(u.y);
    z.set_value(u.z);
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

void VolumeTypeWrapper::set_value(const VolumeType& u, bool do_notify) {
    s0.set_value(u.s[0]);
    s1.set_value(u.s[1]);
    s2.set_value(u.s[2]);
    s3.set_value(u.s[3]);
    s4.set_value(u.s[4]);
    s5.set_value(u.s[5]);
    s6.set_value(u.s[6]);
    s7.set_value(u.s[7]);
}

//----------------------------------------------------------------------------//

void SurfaceWrapper::set_value(const Surface& u, bool do_notify) {
    specular.set_value(u.specular);
    diffuse.set_value(u.diffuse);
}

}  // namespace model