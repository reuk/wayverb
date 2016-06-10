#pragma once

#include "LoadContext.hpp"
#include "WorkQueue.hpp"

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/generic_shader.h"
#include "modern_gl_utils/updatable.h"
#include "modern_gl_utils/vao.h"

class Waveform : public ::Updatable,
                 public ::Drawable,
                 public GLAudioThumbnailBase {
public:
    Waveform(GenericShader& shader);
    void set_position(const glm::vec3& p);

    void update(float dt) override;
    void draw() const override;

    void clear() override;
    void load_from(AudioFormatManager& manager, const File& file) override;

    void reset(int num_channels,
               double sample_rate,
               int64 total_samples) override;
    void addBlock(int64 sample_number_in_source,
                  const AudioSampleBuffer& new_data,
                  int start_offset,
                  int num_samples) override;

    void set_amplitude_scale(float f) override;
    void set_time_scale(float f) override;
    float get_length_in_seconds() const ;

    void set_visible_range(const Range<float>& range) override;

private:
    static const int per_buffer = 16;

    void load_from(std::unique_ptr<AudioFormatReader>&& reader);
    void clear_impl();

    glm::vec3 get_scale() const;
    glm::vec3 get_position() const;

    static std::vector<glm::vec4> compute_colours(
            const std::vector<glm::vec3>& g);

    std::vector<glm::vec3> compute_geometry(
            const std::vector<std::pair<float, float>>& data);

    GenericShader* shader;

    VAO vao;
    DynamicVBO geometry;
    DynamicVBO colors;
    DynamicIBO ibo;

    glm::vec3 position{0};

    int previous_size{0};
    std::vector<std::pair<float, float>> downsampled;

    std::unique_ptr<LoadContext> load_context;
    float x_spacing;

    float amplitude_scale{1};
    float time_scale{1};
    Range<float> visible_range;

    WorkQueue incoming_work_queue;
    mutable std::mutex mut;
};