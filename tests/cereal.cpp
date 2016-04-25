#include "common/json_read_write.h"
#include "common/scene_data.h"
#include "common/surface_owner_serialize.h"

#include "combined_config.h"
#include "combined_config_serialize.h"

#include "gtest/gtest.h"

#include <cereal/types/map.hpp>

#include <glog/logging.h>

#include <string>

TEST(cereal, materials) {
    std::string base_path("/Users/reuben/dev/waveguide/demo/assets/materials");

    for (auto i : {
             "bright.json",
             "brighter.json",
             "damped.json",
             "mat.json",
             "vault.json",
         }) {
        SurfaceLoader surface_loader(base_path + "/" + i);
        LOG(INFO) << "loaded " << i << " successfully";
    }
}

#if 0
TEST(cereal, write_configs) {
    std::string base_path ("/Users/reuben/dev/waveguide/demo/assets/configs");

    auto write_config =
        [&base_path](const std::string& path, const config::Combined& cc) {
            json_read_write::write(base_path + "/" + path,
                                   cereal::make_nvp("config", cc));
        };

    {
        config::Combined cc;
        cc.source = Vec3f(0, 0, 0);
        cc.mic = Vec3f(0, 0, 2);

        auto mic_model = std::make_unique<config::MicrophoneModel>();
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(-1, 0, 1), 0.5));
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(1, 0, 1), 0.5));
        cc.attenuation_model = std::move(mic_model);

        write_config("bedroom.json", cc);
    }

    {
        config::Combined cc;
        cc.source = Vec3f(0, -50, 20);
        cc.mic = Vec3f(-5, -125, 25);

        auto hrtf_model = std::make_unique<config::HrtfModel>();
        hrtf_model->facing = Vec3f(5, 75, -5);
        hrtf_model->up = Vec3f(0, 1, 0);
        cc.attenuation_model = std::move(hrtf_model);

        write_config("config_hrtf.json", cc);
    }

    {
        config::Combined cc;
        cc.source = Vec3f(0, -50, 20);
        cc.mic = Vec3f(-5, -125, 25);

        auto mic_model = std::make_unique<config::MicrophoneModel>();
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(-1, 0, 1), 0.5));
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(1, 0, 1), 0.5));
        cc.attenuation_model = std::move(mic_model);

        write_config("config.json", cc);
    }

    {
        config::Combined cc;
        cc.source = Vec3f(0, 1, 0);
        cc.mic = Vec3f(0, 1, 20);

        auto mic_model = std::make_unique<config::MicrophoneModel>();
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(-1, 0, 1), 0.5));
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(1, 0, 1), 0.5));
        cc.attenuation_model = std::move(mic_model);

        write_config("far_2.json", cc);
    }

    {
        config::Combined cc;
        cc.source = Vec3f(0, 1, 0);
        cc.mic = Vec3f(20, 1, 0);

        auto mic_model = std::make_unique<config::MicrophoneModel>();
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(-1, 0, -1), 0.5));
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(-1, 0, 1), 0.5));
        cc.attenuation_model = std::move(mic_model);

        write_config("far.json", cc);
    }

    {
        config::Combined cc;
        cc.source = Vec3f(0, 1.75, 0);
        cc.mic = Vec3f(0, 1.75, 6);

        auto hrtf_model = std::make_unique<config::HrtfModel>();
        hrtf_model->facing = Vec3f(-1, 0, 0);
        hrtf_model->up = Vec3f(0, 1, 0);
        cc.attenuation_model = std::move(hrtf_model);

        write_config("hrtf_vault_l.json", cc);
    }

    {
        config::Combined cc;
        cc.source = Vec3f(0, 1.75, 0);
        cc.mic = Vec3f(0, 1.75, 6);

        auto hrtf_model = std::make_unique<config::HrtfModel>();
        hrtf_model->facing = Vec3f(1, 0, 0);
        hrtf_model->up = Vec3f(0, 1, 0);
        cc.attenuation_model = std::move(hrtf_model);

        write_config("hrtf_vault_r.json", cc);
    }

    {
        config::Combined cc;
        cc.source = Vec3f(0, 1.75, 0);
        cc.mic = Vec3f(0, 1.75, 6);

        auto hrtf_model = std::make_unique<config::HrtfModel>();
        hrtf_model->facing = Vec3f(0, 0, -1);
        hrtf_model->up = Vec3f(0, 1, 0);
        cc.attenuation_model = std::move(hrtf_model);

        write_config("hrtf_vault.json", cc);
    }

    {
        config::Combined cc;
        cc.source = Vec3f(0, 2, 6);
        cc.mic = Vec3f(0, 2, -6);

        auto mic_model = std::make_unique<config::MicrophoneModel>();
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(-1, 0, -1), 0.5));
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(-1, 0, 1), 0.5));
        cc.attenuation_model = std::move(mic_model);

        write_config("large_square.json", cc);
    }

    {
        config::Combined cc;
        cc.source = Vec3f(0, 1, 0);
        cc.mic = Vec3f(0, 1, 7);

        auto mic_model = std::make_unique<config::MicrophoneModel>();
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(-1, 0, -1), 0.5));
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(-1, 0, 1), 0.5));
        cc.attenuation_model = std::move(mic_model);

        write_config("medium.json", cc);
    }

    {
        config::Combined cc;
        cc.source = Vec3f(0, 1, 0);
        cc.mic = Vec3f(0, 1, 1.5);

        auto mic_model = std::make_unique<config::MicrophoneModel>();
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(-1, 0, -1), 0.5));
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(-1, 0, 1), 0.5));
        cc.attenuation_model = std::move(mic_model);

        write_config("near_c.json", cc);
    }

    {
        config::Combined cc;
        cc.source = Vec3f(0, 1, 0);
        cc.mic = Vec3f(0, 1, 2);

        auto mic_model = std::make_unique<config::MicrophoneModel>();
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(-1, 0, -1), 0.5));
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(-1, 0, 1), 0.5));
        cc.attenuation_model = std::move(mic_model);

        write_config("tunnel.json", cc);
    }

    {
        config::Combined cc;
        cc.source = Vec3f(0, 1.75, 0);
        cc.mic = Vec3f(0, 1.75, 3);

        auto mic_model = std::make_unique<config::MicrophoneModel>();
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(-1, 0, -1), 0.5));
        mic_model->microphones.push_back(
            config::Microphone(Vec3f(-1, 0, 1), 0.5));
        cc.attenuation_model = std::move(mic_model);

        write_config("vault.json", cc);
    }
}
#endif

TEST(cereal, configs) {
    std::string base_path("/Users/reuben/dev/waveguide/demo/assets/configs");

    config::Combined example;
    json_read_write::write(base_path + "/" + "example.json",
                           cereal::make_nvp("config", example));

    for (auto i : {
             "bedroom.json",
             "config_hrtf.json",
             "config.json",
             "far_2.json",
             "far.json",
             "hrtf_vault_l.json",
             "hrtf_vault_r.json",
             "hrtf_vault.json",
             "large_square.json",
             "medium.json",
             "near_c.json",
             "tunnel.json",
             "vault.json",
         }) {
        config::Combined cc;
        json_read_write::read(base_path + "/" + i, cc);
        LOG(INFO) << "loaded " << i << " successfully";
    }
}
