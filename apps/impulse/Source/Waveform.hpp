#pragma once

#include "LoadContext.hpp"
#include "ThumbnailBuffer.hpp"
#include "WorkQueue.hpp"

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/generic_shader.h"
#include "modern_gl_utils/updatable.h"
#include "modern_gl_utils/vao.h"

class Waveform
        : public mglu::Updatable,
          public mglu::Drawable,
          public AudioFormatWriter::ThreadedWriter::IncomingDataReceiver {
public:
    Waveform(mglu::GenericShader& shader,
             AudioFormatManager& manager,
             const File& file);
    void set_position(const glm::vec3& p);

    void update(float dt) override;

    void reset(int num_channels,
               double sample_rate,
               int64 total_samples) override;
    void addBlock(int64 sample_number_in_source,
                  const AudioSampleBuffer& new_data,
                  int start_offset,
                  int num_samples) override;

private:
    void do_draw(const glm::mat4& modelview_matrix) const override;
    glm::mat4 get_local_modelview_matrix() const override;

    void clear_impl();

    glm::vec3 get_position() const;

    static std::vector<glm::vec4> compute_colours(
            const std::vector<glm::vec3>& g);

    std::vector<glm::vec3> compute_geometry(
            const std::vector<std::pair<float, float>>& data);

    mutable std::mutex mut;

    mglu::GenericShader* shader;

    mglu::VAO vao;
    mglu::DynamicVBO geometry;
    mglu::DynamicVBO colors;
    mglu::DynamicIBO ibo;

    glm::vec3 position{0};

    int previous_size{0};
    std::vector<std::pair<float, float>> downsampled;

    InputBufferedHopBuffer<float> input_buffer;

    WorkQueue incoming_work_queue;

    std::unique_ptr<LoadContext> load_context;
    float x_spacing;
};