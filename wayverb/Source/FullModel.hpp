#pragma once

#include "Model.h"
#include "collection.h"

#include "combined/engine.h"

#include "cereal/types/vector.hpp"

namespace model {

template <>
class ValueWrapper<glm::vec3> : public StructWrapper<glm::vec3, 3> {
public:
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
        return {{&x, &y, &z}};
    }
    MODEL_FIELD_DEFINITION(x);
    MODEL_FIELD_DEFINITION(y);
    MODEL_FIELD_DEFINITION(z);
};

template <>
class ValueWrapper<VolumeType> : public StructWrapper<VolumeType, 8> {
public:
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
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
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
        return {{&specular, &diffuse}};
    }
    MODEL_FIELD_DEFINITION(specular);
    MODEL_FIELD_DEFINITION(diffuse);
};

template <>
class ValueWrapper<SceneData::Material>
    : public StructWrapper<SceneData::Material, 2> {
public:
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
        return {{&name, &surface}};
    }
    MODEL_FIELD_DEFINITION(name);
    MODEL_FIELD_DEFINITION(surface);
};

template <>
class ValueWrapper<Orientable::AzEl>
    : public StructWrapper<Orientable::AzEl, 2> {
public:
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
        return {{&azimuth, &elevation}};
    }
    MODEL_FIELD_DEFINITION(azimuth);
    MODEL_FIELD_DEFINITION(elevation);
};

template <>
class ValueWrapper<Pointer> : public StructWrapper<Pointer, 3> {
public:
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
        return {{&mode, &spherical, &look_at}};
    }
    MODEL_FIELD_DEFINITION(mode);
    MODEL_FIELD_DEFINITION(spherical);
    MODEL_FIELD_DEFINITION(look_at);
};

template <>
class ValueWrapper<Microphone> : public StructWrapper<Microphone, 2> {
public:
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
        return {{&pointer, &shape}};
    }
    MODEL_FIELD_DEFINITION(pointer);
    MODEL_FIELD_DEFINITION(shape);
};

template <>
class ValueWrapper<ReceiverSettings>
    : public StructWrapper<ReceiverSettings, 3> {
public:
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
        return {{&mode, &microphones, &hrtf}};
    }
    MODEL_FIELD_DEFINITION(mode);
    MODEL_FIELD_DEFINITION(microphones);
    MODEL_FIELD_DEFINITION(hrtf);
};

template <>
class ValueWrapper<App> : public StructWrapper<App, 6> {
public:
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
        return {{&filter_frequency,
                 &oversample_ratio,
                 &rays,
                 &source,
                 &receiver,
                 &receiver_settings}};
    }
    MODEL_FIELD_DEFINITION(filter_frequency);
    MODEL_FIELD_DEFINITION(oversample_ratio);
    MODEL_FIELD_DEFINITION(rays);
    MODEL_FIELD_DEFINITION(source);
    MODEL_FIELD_DEFINITION(receiver);
    MODEL_FIELD_DEFINITION(receiver_settings);
};

class RenderState {
public:
    bool is_rendering{false};
    engine::State state{engine::State::idle};
    double progress{0};
    bool visualise{true};
};

template <>
class ValueWrapper<RenderState> : public StructWrapper<RenderState, 6> {
public:
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
        return {{&is_rendering, &state, &progress, &visualise}};
    }

    void start() {
        is_rendering.set(true);
    }

    void stop() {
        is_rendering.set(false);
        state.set(engine::State::idle);
        progress.set(0);
    }

    MODEL_FIELD_DEFINITION(is_rendering);
    MODEL_FIELD_DEFINITION(state);
    MODEL_FIELD_DEFINITION(progress);
    MODEL_FIELD_DEFINITION(visualise);
};

class Persistent {
public:
    App app;
    std::vector<SceneData::Material> materials;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("app", app),
                cereal::make_nvp("materials", materials));
    }
};

template <>
class ValueWrapper<Persistent> : public StructWrapper<Persistent, 2> {
public:
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
        return {{&app, &materials}};
    }
    MODEL_FIELD_DEFINITION(app);
    MODEL_FIELD_DEFINITION(materials);
};

class FullModel {
public:
    Persistent persistent;
    std::vector<SceneData::Material> presets;
    RenderState render_state;
    int shown_surface{-1};
    bool needs_save{false};
};

template <>
class ValueWrapper<FullModel> : public StructWrapper<FullModel, 5> {
public:
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
        return {{&persistent,
                 &presets,
                 &render_state,
                 &shown_surface,
                 &needs_save}};
    }
    MODEL_FIELD_DEFINITION(persistent);
    MODEL_FIELD_DEFINITION(presets);
    MODEL_FIELD_DEFINITION(render_state);
    MODEL_FIELD_DEFINITION(shown_surface);
    MODEL_FIELD_DEFINITION(needs_save);
};

}  // namespace model