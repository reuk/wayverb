#pragma once

#include "combined_config.h"
#include "common/scene_data.h"

#include "../JuceLibraryCode/JuceHeader.h"

namespace model {

template <typename Broadcaster>
struct ListenerFunctionTrait {
    template <typename Listener>
    static void add_listener(Broadcaster* const b, Listener* const l) {
        b->addListener(l);
    }

    template <typename Listener>
    static void remove_listener(Broadcaster* const b, Listener* const l) {
        b->removeListener(l);
    }
};

template <>
struct ListenerFunctionTrait<ChangeBroadcaster> {
    template <typename Listener>
    static void add_listener(ChangeBroadcaster* const b, Listener* const l) {
        b->addChangeListener(l);
    }

    template <typename Listener>
    static void remove_listener(ChangeBroadcaster* const b, Listener* const l) {
        b->removeChangeListener(l);
    }
};

template <typename Broadcaster,
          typename Listener = typename Broadcaster::Listener>
class Connector {
public:
    Connector(Broadcaster* const cb, Listener* const cl)
            : cb(cb)
            , cl(cl) {
        if (cb && cl) {
            ListenerFunctionTrait<Broadcaster>::add_listener(cb, cl);
        }
    }

    Connector(const Connector&) = delete;
    Connector& operator=(const Connector&) = delete;
    Connector(Connector&&) noexcept = delete;
    Connector& operator=(Connector&&) noexcept = delete;

    virtual ~Connector() noexcept {
        if (cb && cl) {
            ListenerFunctionTrait<Broadcaster>::remove_listener(cb, cl);
        }
    }

private:
    Broadcaster* const cb;
    Listener* const cl;
};

using ChangeConnector = Connector<ChangeBroadcaster, ChangeListener>;

class ModelMember : public ChangeListener, public ChangeBroadcaster {
public:
    ModelMember(ModelMember* owner);

    ModelMember(const ModelMember&) = delete;
    ModelMember& operator=(const ModelMember&) = delete;
    ModelMember(ModelMember&&) noexcept = delete;
    ModelMember& operator=(ModelMember&&) noexcept = delete;
    virtual ~ModelMember() noexcept = default;

    void changeListenerCallback(ChangeBroadcaster* cb) override;
    void notify();

    ModelMember* get_owner() const;

private:
    ModelMember* owner;
    ChangeConnector owner_connector{this, owner};
};

template <typename T>
class ModelValue : public ModelMember {
public:
    using ModelMember::ModelMember;

    virtual const T& get_value() const = 0;
    virtual void set_value(const T& u, bool do_notify) = 0;
};

template <typename T>
class ValueWrapper : public ModelValue<T> {
public:
    ValueWrapper(ModelMember* owner, T& t)
            : ModelValue<T>(owner)
            , t(t) {
    }

    const T& get_value() const override {
        return t;
    }

    void set_value(const T& u, bool do_notify = true) override {
        t = u;
        if (do_notify) {
            ModelMember::notify();
        }
    }

private:
    T& t;
};

template <typename T, size_t values>
class NestedValueWrapper : public ModelValue<T> {
public:
    NestedValueWrapper(ModelMember* owner, T& t)
            : ModelValue<T>(owner)
            , t(t) {
    }

    const T& get_value() const override {
        return t;
    }

protected:
    T& t;
};

template <>
class ValueWrapper<Vec3f> : public NestedValueWrapper<Vec3f, 3> {
public:
    using NestedValueWrapper<Vec3f, 3>::NestedValueWrapper;

    void set_value(const Vec3f& u, bool do_notify = true) override {
        x.set_value(u.x, do_notify);
        y.set_value(u.y, do_notify);
        z.set_value(u.z, do_notify);
    }

    ValueWrapper<float> x{this, t.x};
    ValueWrapper<float> y{this, t.y};
    ValueWrapper<float> z{this, t.z};
};

template <>
class ValueWrapper<config::Combined>
    : public NestedValueWrapper<config::Combined, 14> {
public:
    using NestedValueWrapper<config::Combined, 14>::NestedValueWrapper;

    void set_value(const config::Combined& u, bool do_notify = true) override {
        filter_frequency.set_value(u.filter_frequency, do_notify);
        oversample_ratio.set_value(u.oversample_ratio, do_notify);
        rays.set_value(u.rays, do_notify);
        impulses.set_value(u.impulses, do_notify);
        ray_hipass.set_value(u.ray_hipass, do_notify);
        do_normalize.set_value(u.do_normalize, do_notify);
        trim_predelay.set_value(u.trim_predelay, do_notify);
        trim_tail.set_value(u.trim_tail, do_notify);
        remove_direct.set_value(u.remove_direct, do_notify);
        volume_scale.set_value(u.volume_scale, do_notify);
        source.set_value(u.source, do_notify);
        mic.set_value(u.mic, do_notify);
        sample_rate.set_value(u.sample_rate, do_notify);
        bit_depth.set_value(u.bit_depth, do_notify);
    }

    ValueWrapper<float> filter_frequency{this, t.filter_frequency};
    ValueWrapper<float> oversample_ratio{this, t.oversample_ratio};
    ValueWrapper<int> rays{this, t.rays};
    ValueWrapper<int> impulses{this, t.impulses};
    ValueWrapper<float> ray_hipass{this, t.ray_hipass};
    ValueWrapper<bool> do_normalize{this, t.do_normalize};
    ValueWrapper<bool> trim_predelay{this, t.trim_predelay};
    ValueWrapper<bool> trim_tail{this, t.trim_tail};
    ValueWrapper<bool> remove_direct{this, t.remove_direct};
    ValueWrapper<float> volume_scale{this, t.volume_scale};
    ValueWrapper<Vec3f> source{this, t.source};
    ValueWrapper<Vec3f> mic{this, t.mic};
    ValueWrapper<float> sample_rate{this, t.sample_rate};
    ValueWrapper<int> bit_depth{this, t.bit_depth};
};

template <>
class ValueWrapper<VolumeType> : public NestedValueWrapper<VolumeType, 8> {
public:
    using NestedValueWrapper<VolumeType, 8>::NestedValueWrapper;

    void set_value(const VolumeType& u, bool do_notify = true) override {
        s0.set_value(u.s0, do_notify);
        s1.set_value(u.s1, do_notify);
        s2.set_value(u.s2, do_notify);
        s3.set_value(u.s3, do_notify);
        s4.set_value(u.s4, do_notify);
        s5.set_value(u.s5, do_notify);
        s6.set_value(u.s6, do_notify);
        s7.set_value(u.s7, do_notify);
    }

    ValueWrapper<float> s0{this, t.s[0]};
    ValueWrapper<float> s1{this, t.s[1]};
    ValueWrapper<float> s2{this, t.s[2]};
    ValueWrapper<float> s3{this, t.s[3]};
    ValueWrapper<float> s4{this, t.s[4]};
    ValueWrapper<float> s5{this, t.s[5]};
    ValueWrapper<float> s6{this, t.s[6]};
    ValueWrapper<float> s7{this, t.s[7]};
};

template <>
class ValueWrapper<Surface> : public NestedValueWrapper<Surface, 2> {
public:
    using NestedValueWrapper<Surface, 2>::NestedValueWrapper;

    void set_value(const Surface& u, bool do_notify = true) override {
        specular.set_value(u.specular, do_notify);
        diffuse.set_value(u.diffuse, do_notify);
    }

    ValueWrapper<VolumeType> specular{this, t.specular};
    ValueWrapper<VolumeType> diffuse{this, t.diffuse};
};

template <>
class ValueWrapper<SceneData::Material>
    : public NestedValueWrapper<SceneData::Material, 2> {
public:
    using NestedValueWrapper<SceneData::Material, 2>::NestedValueWrapper;

    void set_value(const SceneData::Material& u,
                   bool do_notify = true) override {
        name.set_value(u.name, do_notify);
        surface.set_value(u.surface, do_notify);
    }

    ValueWrapper<std::string> name{this, t.name};
    ValueWrapper<Surface> surface{this, t.surface};
};

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

    const T& get_value() const {
        return wrapper.get_value();
    }

    void set_value(const T& u, bool do_notify = true) {
        wrapper.set_value(u, do_notify);
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