#pragma once

#include "FadeShader.hpp"
#include "LoadContext.hpp"
#include "WorkQueue.hpp"

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/updatable.h"
#include "modern_gl_utils/vao.h"

class Waterfall : public ::Updatable,
                  public ::Drawable,
                  public GLAudioThumbnailBase {
public:
    enum class Mode { linear, log };

    Waterfall(const FadeShader& shader);

    /*
        Waterfall(const FadeShader& shader, const std::vector<float>& signal)
                : Waterfall(shader,
                            Spectrogram(window, hop).compute(signal),
                            spacing,
                            2.0) {
        }
    */

    void update(float dt) override;
    void draw() const override;

    void set_mode(Mode u);

    void set_position(const glm::vec3& p);

    void clear() override;
    void load_from(AudioFormatManager& manager, const File& file) override;
    void reset(int num_channels,
               double sample_rate,
               int64 total_samples) override;
    void addBlock(int64 sample_number_in_source,
                  const AudioSampleBuffer& new_data,
                  int start_offset,
                  int num_samples) override;

private:
    void load_from(std::unique_ptr<AudioFormatReader>&& reader);
    void clear_impl();

    /*
        Waterfall(const FadeShader& shader,
                  const std::vector<std::vector<float>>& heights,
                  float x_spacing,
                  float z_width)
                : shader(shader)
                , spectrogram(heights)
                , x_spacing(x_spacing)
                , z_width(z_width)
                , strips(compute_strips(
                      shader, spectrogram, mode, x_spacing, z_width)) {
        }
    */

    class HeightMapStrip : public ::Drawable {
    public:
        HeightMapStrip(const FadeShader& shader,
                       const std::vector<float>& left,
                       const std::vector<float>& right,
                       Mode mode,
                       float x,
                       float x_spacing,
                       float z_width);

        void draw() const override;

    private:
        static std::vector<glm::vec3> compute_geometry(
                const std::vector<float>& left,
                const std::vector<float>& right,
                Mode mode,
                float x,
                float x_spacing,
                float z_width);

        static glm::vec3 compute_mapped_colour(float r);

        static std::vector<glm::vec4> compute_colors(
                const std::vector<glm::vec3>& g);

        const FadeShader& shader;

        VAO vao;
        StaticVBO geometry;
        StaticVBO colors;
        StaticIBO ibo;
        GLuint size;
    };

    static std::vector<HeightMapStrip> compute_strips(
            const FadeShader& shader,
            const std::vector<std::vector<float>>& input,
            Mode mode,
            float x_spacing,
            float z_width);

    static const int per_buffer{4};

    const FadeShader& shader;

    Mode mode{Mode::log};

    std::vector<std::vector<float>> spectrum;

    glm::vec3 position{0};

    std::vector<HeightMapStrip> strips;

    std::unique_ptr<LoadContext> load_context;
    float x_spacing;
    float z_width{2};

    WorkQueue incoming_work_queue;

    mutable std::mutex mut;
};
