#pragma once

#include "combined_config.h"
#include "common/scene_data.h"

#include "../JuceLibraryCode/JuceHeader.h"

namespace model {

class ChangeConnector {
public:
    ChangeConnector(ChangeBroadcaster* cb, ChangeListener* cl);
    virtual ~ChangeConnector() noexcept;

private:
    ChangeBroadcaster* cb;
    ChangeListener* cl;
};

class Model : public ChangeListener, public ChangeBroadcaster {
public:
    Model() = default;
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&&) noexcept = delete;
    Model& operator=(Model&&) noexcept = delete;
    virtual ~Model() noexcept = default;

    void changeListenerCallback(ChangeBroadcaster* cb) override;
    void notify();
};

class ModelMember : public Model {
public:
    ModelMember(ModelMember* parent);

    ModelMember(const ModelMember&) = delete;
    ModelMember& operator=(const ModelMember&) = delete;
    ModelMember(ModelMember&&) noexcept = delete;
    ModelMember& operator=(ModelMember&&) noexcept = delete;
    virtual ~ModelMember() noexcept = default;

    ModelMember* get_parent() const;

private:
    ModelMember* parent;
    ChangeConnector parent_connector{this, parent};
};

template <typename T>
class ValueWrapper : public ModelMember {
public:
    ValueWrapper(ModelMember* parent, T& t)
            : ModelMember(parent)
            , t(t) {
    }

    const T& get_value() const {
        return t;
    }

    void set_value(const T& u) {
        t = u;
        notify();
    }

protected:
    T& t;
};

class Vec3fWrapper : public ValueWrapper<Vec3f> {
public:
    using ValueWrapper<Vec3f>::ValueWrapper;

    ValueWrapper<float> x{this, t.x};
    ValueWrapper<float> y{this, t.y};
    ValueWrapper<float> z{this, t.z};
};

class Combined : public ModelMember {
public:
    //  set model parent
    Combined(ModelMember* parent,
             const config::Combined& rhs = config::Combined());

private:
    config::Combined data;

public:
    ValueWrapper<float> filter_frequency{this, data.filter_frequency};
    ValueWrapper<float> oversample_ratio{this, data.oversample_ratio};
    ValueWrapper<int> rays{this, data.rays};
    ValueWrapper<int> impulses{this, data.impulses};
    ValueWrapper<float> ray_hipass{this, data.ray_hipass};
    ValueWrapper<bool> do_normalize{this, data.do_normalize};
    ValueWrapper<bool> trim_predelay{this, data.trim_predelay};
    ValueWrapper<bool> trim_tail{this, data.trim_tail};
    ValueWrapper<bool> remove_direct{this, data.remove_direct};
    ValueWrapper<float> volume_scale{this, data.volume_scale};
    Vec3fWrapper source{this, data.source};
    Vec3fWrapper mic{this, data.mic};
    ValueWrapper<float> sample_rate{this, data.sample_rate};
    ValueWrapper<int> bit_depth{this, data.bit_depth};

    const config::Combined& get_data() const;
};

class VolumeTypeWrapper : public ValueWrapper<VolumeType> {
public:
    using ValueWrapper<VolumeType>::ValueWrapper;

    ValueWrapper<float> s0{this, t.s[0]};
    ValueWrapper<float> s1{this, t.s[1]};
    ValueWrapper<float> s2{this, t.s[2]};
    ValueWrapper<float> s3{this, t.s[3]};
    ValueWrapper<float> s4{this, t.s[4]};
    ValueWrapper<float> s5{this, t.s[5]};
    ValueWrapper<float> s6{this, t.s[6]};
    ValueWrapper<float> s7{this, t.s[7]};
};

class SurfaceWrapper : public ValueWrapper<Surface> {
public:
    using ValueWrapper<Surface>::ValueWrapper;

    VolumeTypeWrapper specular{this, t.specular};
    VolumeTypeWrapper diffuse{this, t.diffuse};
};

}  // namespace model