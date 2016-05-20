#pragma once

#include "collection.h"

#include "combined/config.h"
#include "combined/engine.h"

namespace model {

template <>
class ValueWrapper<Vec3f> : public StructWrapper<Vec3f, 3> {
public:
    using StructWrapper<Vec3f, 3>::StructWrapper;
    std::array<StructAccessor<Vec3f> *, 3> get_members() override {
        return {{&x, &y, &z}};
    }
    MODEL_FIELD_DEFINITION(x);
    MODEL_FIELD_DEFINITION(y);
    MODEL_FIELD_DEFINITION(z);
};

template <>
class ValueWrapper<VolumeType> : public StructWrapper<VolumeType, 8> {
public:
    using StructWrapper<VolumeType, 8>::StructWrapper;
    std::array<StructAccessor<VolumeType> *, 8> get_members() override {
        return {{&s0, &s1, &s2, &s3, &s4, &s5, &s6, &s7}};
    }
    RENAMED_FIELD_DEFINITION(s[0], s0);
    RENAMED_FIELD_DEFINITION(s[1], s1);
    RENAMED_FIELD_DEFINITION(s[2], s2);
    RENAMED_FIELD_DEFINITION(s[3], s3);
    RENAMED_FIELD_DEFINITION(s[4], s4);
    RENAMED_FIELD_DEFINITION(s[5], s5);
    RENAMED_FIELD_DEFINITION(s[6], s6);
    RENAMED_FIELD_DEFINITION(s[7], s7);
};

template <>
class ValueWrapper<Surface> : public StructWrapper<Surface, 2> {
public:
    using StructWrapper<Surface, 2>::StructWrapper;
    std::array<StructAccessor<Surface> *, 2> get_members() override {
        return {{&specular, &diffuse}};
    }
    MODEL_FIELD_DEFINITION(specular);
    MODEL_FIELD_DEFINITION(diffuse);
};

template <>
class ValueWrapper<SceneData::Material>
    : public StructWrapper<SceneData::Material, 2> {
public:
    using StructWrapper<SceneData::Material, 2>::StructWrapper;
    std::array<StructAccessor<SceneData::Material> *, 2> get_members()
        override {
        return {{&name, &surface}};
    }
    MODEL_FIELD_DEFINITION(name);
    MODEL_FIELD_DEFINITION(surface);
};

template <>
class ValueWrapper<config::Combined>
    : public StructWrapper<config::Combined, 14> {
public:
    using StructWrapper<config::Combined, 14>::StructWrapper;
    std::array<StructAccessor<config::Combined> *, 14> get_members() override {
        return {{&filter_frequency,
                 &oversample_ratio,
                 &rays,
                 &impulses,
                 &ray_hipass,
                 &do_normalize,
                 &trim_predelay,
                 &trim_tail,
                 &remove_direct,
                 &volume_scale,
                 &source,
                 &mic,
                 &sample_rate,
                 &bit_depth}};
    }
    MODEL_FIELD_DEFINITION(filter_frequency);
    MODEL_FIELD_DEFINITION(oversample_ratio);
    MODEL_FIELD_DEFINITION(rays);
    MODEL_FIELD_DEFINITION(impulses);
    MODEL_FIELD_DEFINITION(ray_hipass);
    MODEL_FIELD_DEFINITION(do_normalize);
    MODEL_FIELD_DEFINITION(trim_predelay);
    MODEL_FIELD_DEFINITION(trim_tail);
    MODEL_FIELD_DEFINITION(remove_direct);
    MODEL_FIELD_DEFINITION(volume_scale);
    MODEL_FIELD_DEFINITION(source);
    MODEL_FIELD_DEFINITION(mic);
    MODEL_FIELD_DEFINITION(sample_rate);
    MODEL_FIELD_DEFINITION(bit_depth);
};

struct FullReceiverConfig {
    config::AttenuationModel::Mode mode;
    config::MicrophoneModel microphone_model;
    config::HrtfModel hrtf_model;
};

template <>
class ValueWrapper<config::Microphone>
    : public StructWrapper<config::Microphone, 2> {
public:
    using StructWrapper<config::Microphone, 2>::StructWrapper;
    std::array<StructAccessor<config::Microphone> *, 2> get_members() override {
        return {{&facing, &shape}};
    }
    MODEL_FIELD_DEFINITION(facing);
    MODEL_FIELD_DEFINITION(shape);
};

template <>
class ValueWrapper<config::MicrophoneModel>
    : public StructWrapper<config::MicrophoneModel, 1> {
public:
    using StructWrapper<config::MicrophoneModel, 1>::StructWrapper;
    std::array<StructAccessor<config::MicrophoneModel> *, 1> get_members()
        override {
        return {{&microphones}};
    }
    MODEL_FIELD_DEFINITION(microphones);
};

template <>
class ValueWrapper<config::HrtfModel>
    : public StructWrapper<config::HrtfModel, 2> {
public:
    using StructWrapper<config::HrtfModel, 2>::StructWrapper;
    std::array<StructAccessor<config::HrtfModel> *, 2> get_members() override {
        return {{&facing, &up}};
    }
    MODEL_FIELD_DEFINITION(facing);
    MODEL_FIELD_DEFINITION(up);
};

template <>
class ValueWrapper<FullReceiverConfig>
    : public StructWrapper<FullReceiverConfig, 3> {
public:
    using StructWrapper<FullReceiverConfig, 3>::StructWrapper;
    std::array<StructAccessor<FullReceiverConfig> *, 3> get_members() override {
        return {{&mode, &microphone_model, &hrtf_model}};
    }
    MODEL_FIELD_DEFINITION(mode);
    MODEL_FIELD_DEFINITION(microphone_model);
    MODEL_FIELD_DEFINITION(hrtf_model);
};

class RenderState {
public:
    bool is_rendering{false};
    engine::State state{engine::State::idle};
    double progress{0};
    bool show_waveguide{true};
    bool show_raytracer{true};
};

template <>
class ValueWrapper<RenderState> : public StructWrapper<RenderState, 5> {
public:
    using StructWrapper<RenderState, 5>::StructWrapper;
    std::array<StructAccessor<RenderState> *, 5> get_members() override {
        return {{&is_rendering,
                 &state,
                 &progress,
                 &show_waveguide,
                 &show_raytracer}};
    }
    MODEL_FIELD_DEFINITION(is_rendering);
    MODEL_FIELD_DEFINITION(state);
    MODEL_FIELD_DEFINITION(progress);
    MODEL_FIELD_DEFINITION(show_waveguide);
    MODEL_FIELD_DEFINITION(show_raytracer);
};

class FullModel {
public:
    config::Combined combined;
    std::vector<SceneData::Material> materials;
    std::vector<SceneData::Material> presets;
    FullReceiverConfig receiver;
    RenderState render_state;
};

template <>
class ValueWrapper<FullModel> : public StructWrapper<FullModel, 5> {
public:
    using StructWrapper<FullModel, 5>::StructWrapper;
    std::array<StructAccessor<FullModel> *, 5> get_members() override {
        return {{&combined, &materials, &presets, &receiver, &render_state}};
    }
    MODEL_FIELD_DEFINITION(combined);
    MODEL_FIELD_DEFINITION(materials);
    MODEL_FIELD_DEFINITION(presets);
    MODEL_FIELD_DEFINITION(receiver);
    MODEL_FIELD_DEFINITION(render_state);
};

}  // namespace model