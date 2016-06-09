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
    Waveform(const GenericShader& shader);
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

private:
    static const int per_buffer = 16;

    void load_from(std::unique_ptr<AudioFormatReader>&& reader);
    void clear_impl();

    static std::vector<glm::vec4> compute_colours(
            const std::vector<glm::vec3>& g);

    std::vector<glm::vec3> compute_geometry(
            const std::vector<std::pair<float, float>>& data);

    const GenericShader& shader;

    VAO vao;
    DynamicVBO geometry;
    DynamicVBO colors;
    DynamicIBO ibo;

    size_t previous_size{0};
    std::vector<std::pair<float, float>> downsampled;

    std::unique_ptr<LoadContext> load_context;
    float spacing;

    glm::vec3 position{0};

    WorkQueue incoming_work_queue;

    mutable std::mutex mut;
};