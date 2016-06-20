#pragma once

#include "FadeShader.hpp"
#include "FrequencyAxis.hpp"
#include "LoadContext.hpp"
#include "Spectrogram.hpp"
#include "WaterfallShader.hpp"
#include "WorkQueue.hpp"

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/updatable.h"
#include "modern_gl_utils/vao.h"

class Waterfall
        : public mglu::Updatable,
          public mglu::Drawable,
          public AudioFormatWriter::ThreadedWriter::IncomingDataReceiver {
public:
    enum class Mode { linear, log };

    Waterfall(WaterfallShader& waterfall_shader,
              FadeShader& fade_shader,
              TexturedQuadShader& quad_shader,
              AudioFormatManager& manager,
              const File& file);

    void update(float dt) override;

    void set_mode(Mode u);

    void set_position(const glm::vec3& p);

    void reset(int num_channels,
               double sample_rate,
               int64 total_samples) override;
    void addBlock(int64 sample_number_in_source,
                  const AudioSampleBuffer& new_data,
                  int start_offset,
                  int num_samples) override;

    void set_visible_range(const Range<double>& range);

    static float z_to_frequency(Mode mode, float z);
    static float frequency_to_z(Mode mode, float frequency);

    static const float width;

private:
    void do_draw(const glm::mat4& modelview_matrix) const override;
    glm::mat4 get_local_modelview_matrix() const override;

    void clear_impl();

    glm::vec3 get_scale() const;

    class HeightMapStrip : public mglu::Drawable {
    public:
        HeightMapStrip(WaterfallShader& shader,
                       const std::vector<float>& left,
                       const std::vector<float>& right,
                       Mode mode,
                       float x,
                       float x_spacing,
                       float min_frequency,
                       float max_frequency,
                       float sample_rate);

    private:
        void do_draw(const glm::mat4& modelview_matrix) const override;
        glm::mat4 get_local_modelview_matrix() const override;

        static std::vector<glm::vec3> compute_geometry(
                const std::vector<float>& left,
                const std::vector<float>& right,
                Mode mode,
                float x,
                float x_spacing,
                float min_frequency,
                float max_frequency,
                float sample_rate);

        WaterfallShader* shader;

        mglu::VAO vao;
        mglu::StaticVBO geometry;
        mglu::StaticIBO ibo;
        GLuint size;
    };

    static const int axes{6};

    static const float min_frequency;
    static const float max_frequency;

    mutable std::mutex mut;

    WaterfallShader* waterfall_shader;
    FadeShader* fade_shader;
    TexturedQuadShader* quad_shader;

    glm::vec3 position{0};

    Mode mode{Mode::log};
    std::vector<std::vector<float>> spectrum;
    std::vector<HeightMapStrip> strips;

    std::vector<std::unique_ptr<AxisObject>> frequency_axis_objects;
    std::vector<std::unique_ptr<AxisObject>> time_axis_objects;

    Range<double> visible_range;
    float time_axis_interval{1};

    BufferedSpectrogram spectrogram;

    WorkQueue incoming_work_queue;

    std::unique_ptr<LoadContext> load_context;
    float x_spacing;
};