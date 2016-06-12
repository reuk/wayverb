#pragma once

#include "FadeShader.hpp"
#include "FrequencyAxis.hpp"
#include "LoadContext.hpp"
#include "WaterfallShader.hpp"
#include "WorkQueue.hpp"

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/updatable.h"
#include "modern_gl_utils/vao.h"

class Waterfall : public ::Updatable,
                  public ::Drawable,
                  public ::MatrixTreeNode,
                  public GLAudioThumbnailBase {
public:
    enum class Mode { linear, log };

    Waterfall(MatrixTreeNode* parent, WaterfallShader& waterfall_shader,
              FadeShader& fade_shader,
              TexturedQuadShader& quad_shader);

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

    static float z_to_frequency(Mode mode, float z);
    static float frequency_to_z(Mode mode, float frequency);

    static const float width;

private:
    glm::mat4 get_local_modelview_matrix() const override;

    void load_from(std::unique_ptr<AudioFormatReader>&& reader);
    void clear_impl();

    glm::vec3 get_scale() const;

    class HeightMapStrip : public ::Drawable , public MatrixTreeNode {
    public:
        HeightMapStrip(MatrixTreeNode* parent, WaterfallShader& shader,
                       const std::vector<float>& left,
                       const std::vector<float>& right,
                       Mode mode,
                       float x,
                       float x_spacing,
                       float min_frequency,
                       float max_frequency,
                       float sample_rate);

        void draw() const override;

        glm::mat4 get_local_modelview_matrix() const override;

    private:
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

        VAO vao;
        StaticVBO geometry;
        StaticIBO ibo;
        GLuint size;
    };

    static const int per_buffer{4};
    static const int axes{6};

    static const float min_frequency;
    static const float max_frequency;

    WaterfallShader* waterfall_shader;
    FadeShader* fade_shader;
    TexturedQuadShader* quad_shader;

    glm::vec3 position{0};

    Mode mode{Mode::log};
    std::vector<std::vector<float>> spectrum;
    std::vector<HeightMapStrip> strips;

    std::unique_ptr<LoadContext> load_context;
    float x_spacing;

    WorkQueue incoming_work_queue;
    mutable std::mutex mut;
};
