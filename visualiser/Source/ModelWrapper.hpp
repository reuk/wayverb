#pragma once

#include "Collection.h"

#include "combined_config.h"
#include "common/scene_data.h"

namespace model {

template <>
class ValueWrapper<Vec3f> : public NestedValueWrapper<Vec3f> {
public:
    using NestedValueWrapper<Vec3f>::NestedValueWrapper;

protected:
    void set_value(const Vec3f& u, bool do_notify = true) override {
        x.set(u.x, do_notify);
        y.set(u.y, do_notify);
        z.set(u.z, do_notify);
    }

    void reseat_value(Vec3f& u) override {
        x.reseat(u.x);
        y.reseat(u.y);
        z.reseat(u.z);
    }

public:
    ValueWrapper<float> x{this, t->x};
    ValueWrapper<float> y{this, t->y};
    ValueWrapper<float> z{this, t->z};
};

template <>
class ValueWrapper<config::Combined>
    : public NestedValueWrapper<config::Combined> {
public:
    using NestedValueWrapper<config::Combined>::NestedValueWrapper;

protected:
    void set_value(const config::Combined& u, bool do_notify = true) override {
        filter_frequency.set(u.filter_frequency, do_notify);
        oversample_ratio.set(u.oversample_ratio, do_notify);
        rays.set(u.rays, do_notify);
        impulses.set(u.impulses, do_notify);
        ray_hipass.set(u.ray_hipass, do_notify);
        do_normalize.set(u.do_normalize, do_notify);
        trim_predelay.set(u.trim_predelay, do_notify);
        trim_tail.set(u.trim_tail, do_notify);
        remove_direct.set(u.remove_direct, do_notify);
        volume_scale.set(u.volume_scale, do_notify);
        source.set(u.source, do_notify);
        mic.set(u.mic, do_notify);
        sample_rate.set(u.sample_rate, do_notify);
        bit_depth.set(u.bit_depth, do_notify);
    }

    void reseat_value(config::Combined& u) override {
        filter_frequency.reseat(u.filter_frequency);
        oversample_ratio.reseat(u.oversample_ratio);
        rays.reseat(u.rays);
        impulses.reseat(u.impulses);
        ray_hipass.reseat(u.ray_hipass);
        do_normalize.reseat(u.do_normalize);
        trim_predelay.reseat(u.trim_predelay);
        trim_tail.reseat(u.trim_tail);
        remove_direct.reseat(u.remove_direct);
        volume_scale.reseat(u.volume_scale);
        source.reseat(u.source);
        mic.reseat(u.mic);
        sample_rate.reseat(u.sample_rate);
        bit_depth.reseat(u.bit_depth);
    }

public:
    ValueWrapper<float> filter_frequency{this, t->filter_frequency};
    ValueWrapper<float> oversample_ratio{this, t->oversample_ratio};
    ValueWrapper<int> rays{this, t->rays};
    ValueWrapper<int> impulses{this, t->impulses};
    ValueWrapper<float> ray_hipass{this, t->ray_hipass};
    ValueWrapper<bool> do_normalize{this, t->do_normalize};
    ValueWrapper<bool> trim_predelay{this, t->trim_predelay};
    ValueWrapper<bool> trim_tail{this, t->trim_tail};
    ValueWrapper<bool> remove_direct{this, t->remove_direct};
    ValueWrapper<float> volume_scale{this, t->volume_scale};
    ValueWrapper<Vec3f> source{this, t->source};
    ValueWrapper<Vec3f> mic{this, t->mic};
    ValueWrapper<float> sample_rate{this, t->sample_rate};
    ValueWrapper<int> bit_depth{this, t->bit_depth};
};

template <>
class ValueWrapper<VolumeType> : public NestedValueWrapper<VolumeType> {
public:
    using NestedValueWrapper<VolumeType>::NestedValueWrapper;

protected:
    void set_value(const VolumeType& u, bool do_notify = true) override {
        s0.set(u.s0, do_notify);
        s1.set(u.s1, do_notify);
        s2.set(u.s2, do_notify);
        s3.set(u.s3, do_notify);
        s4.set(u.s4, do_notify);
        s5.set(u.s5, do_notify);
        s6.set(u.s6, do_notify);
        s7.set(u.s7, do_notify);
    }

    void reseat_value(VolumeType& u) override {
        s0.reseat(u.s0);
        s1.reseat(u.s1);
        s2.reseat(u.s2);
        s3.reseat(u.s3);
        s4.reseat(u.s4);
        s5.reseat(u.s5);
        s6.reseat(u.s6);
        s7.reseat(u.s7);
    }

public:
    ValueWrapper<float> s0{this, t->s[0]};
    ValueWrapper<float> s1{this, t->s[1]};
    ValueWrapper<float> s2{this, t->s[2]};
    ValueWrapper<float> s3{this, t->s[3]};
    ValueWrapper<float> s4{this, t->s[4]};
    ValueWrapper<float> s5{this, t->s[5]};
    ValueWrapper<float> s6{this, t->s[6]};
    ValueWrapper<float> s7{this, t->s[7]};
};

template <>
class ValueWrapper<Surface> : public NestedValueWrapper<Surface> {
public:
    using NestedValueWrapper<Surface>::NestedValueWrapper;

protected:
    void set_value(const Surface& u, bool do_notify = true) override {
        specular.set(u.specular, do_notify);
        diffuse.set(u.diffuse, do_notify);
    }

    void reseat_value(Surface& u) override {
        specular.reseat(u.specular);
        diffuse.reseat(u.diffuse);
    }

public:
    ValueWrapper<VolumeType> specular{this, t->specular};
    ValueWrapper<VolumeType> diffuse{this, t->diffuse};
};

template <>
class ValueWrapper<SceneData::Material>
    : public NestedValueWrapper<SceneData::Material> {
public:
    using NestedValueWrapper<SceneData::Material>::NestedValueWrapper;

protected:
    void set_value(const SceneData::Material& u,
                   bool do_notify = true) override {
        name.set(u.name, do_notify);
        surface.set(u.surface, do_notify);
    }

    void reseat_value(SceneData::Material& u) override {
        name.reseat(u.name);
        surface.reseat(u.surface);
    }

public:
    ValueWrapper<std::string> name{this, t->name};
    ValueWrapper<Surface> surface{this, t->surface};
};

template <>
class ValueWrapper<config::Microphone>
    : public NestedValueWrapper<config::Microphone> {
public:
    using NestedValueWrapper<config::Microphone>::NestedValueWrapper;

protected:
    void set_value(const config::Microphone& u,
                   bool do_notify = true) override {
        facing.set(u.facing, do_notify);
        shape.set(u.shape, do_notify);
    }

    void reseat_value(config::Microphone& u) override {
        facing.reseat(u.facing);
        shape.reseat(u.shape);
    }

public:
    ValueWrapper<Vec3f> facing{this, t->facing};
    ValueWrapper<float> shape{this, t->shape};
};

template <>
class ValueWrapper<config::MicrophoneModel>
    : public NestedValueWrapper<config::MicrophoneModel> {
public:
    using NestedValueWrapper<config::MicrophoneModel>::NestedValueWrapper;

protected:
    void set_value(const config::MicrophoneModel& u,
                   bool do_notify = true) override {
        microphones.set(u.microphones, do_notify);
    }

    void reseat_value(config::MicrophoneModel& u) override {
        microphones.reseat(u.microphones);
    }

public:
    ValueWrapper<std::vector<config::Microphone>> microphones{this,
                                                              t->microphones};
};

template <>
class ValueWrapper<config::HrtfModel>
    : public NestedValueWrapper<config::HrtfModel> {
public:
    using NestedValueWrapper<config::HrtfModel>::NestedValueWrapper;

protected:
    void set_value(const config::HrtfModel& u, bool do_notify = true) override {
        facing.set(u.facing, do_notify);
        up.set(u.up, do_notify);
    }

    void reseat_value(config::HrtfModel& u) override {
        facing.reseat(u.facing);
        up.reseat(u.up);
    }

public:
    ValueWrapper<Vec3f> facing{this, t->facing};
    ValueWrapper<Vec3f> up{this, t->up};
};

//----------------------------------------------------------------------------//

class FullReceiverConfig {
public:
    config::AttenuationModel::Mode mode;
    config::MicrophoneModel microphone_model;
    config::HrtfModel hrtf_model;
};

template <>
class ValueWrapper<FullReceiverConfig>
    : public NestedValueWrapper<FullReceiverConfig> {
public:
    using NestedValueWrapper<FullReceiverConfig>::NestedValueWrapper;

protected:
    void set_value(const FullReceiverConfig& u,
                   bool do_notify = true) override {
        mode.set(u.mode, do_notify);
        microphone_model.set(u.microphone_model, do_notify);
        hrtf_model.set(u.hrtf_model, do_notify);
    }

    void reseat_value(FullReceiverConfig& u) override {
        mode.reseat(u.mode);
        microphone_model.reseat(u.microphone_model);
        hrtf_model.reseat(u.hrtf_model);
    }

public:
    ValueWrapper<config::AttenuationModel::Mode> mode{this, t->mode};
    ValueWrapper<config::MicrophoneModel> microphone_model{this,
                                                           t->microphone_model};
    ValueWrapper<config::HrtfModel> hrtf_model{this, t->hrtf_model};
};

//----------------------------------------------------------------------------//

template <typename T>
class ValueWithWrapper : public ModelMember {
private:
    T t;
    ValueWrapper<T> wrapper{this, t};

public:
    ValueWithWrapper(ModelMember* owner, const T& t = T())
            : ModelMember(owner)
            , t(t) {
    }

    ValueWithWrapper(ModelMember* owner, T&& t)
            : ModelMember(owner)
            , t(std::move(t)) {
    }

    ValueWithWrapper(const ValueWithWrapper&) = delete;
    ValueWithWrapper& operator=(const ValueWithWrapper&) = delete;
    ValueWithWrapper(ValueWithWrapper&&) noexcept = delete;
    ValueWithWrapper& operator=(ValueWithWrapper&&) noexcept = delete;

    virtual ~ValueWithWrapper() noexcept = default;

    T get_value() const {
        return wrapper;
    }

    void set_value(const T& u, bool do_notify = true) {
        wrapper.set(u, do_notify);
    }

    const ValueWrapper<T>& get_wrapper() const {
        return wrapper;
    }

    ValueWrapper<T>& get_wrapper() {
        return wrapper;
    }
};

using Combined = ValueWithWrapper<config::Combined>;

}  // namespace model