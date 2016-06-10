#include "Waveform.hpp"

#include "ComputeIndices.hpp"

#include "common/stl_wrappers.h"

#include "glm/gtx/transform.hpp"

Waveform::Waveform(GenericShader& shader)
        : shader(&shader) {
    auto s_vao = vao.get_scoped();

    geometry.bind();
    auto v_pos = shader.get_attrib_location("v_position");
    glEnableVertexAttribArray(v_pos);
    glVertexAttribPointer(v_pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    colors.bind();
    auto c_pos = shader.get_attrib_location("v_color");
    glEnableVertexAttribArray(c_pos);
    glVertexAttribPointer(c_pos, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    ibo.bind();
}

void Waveform::set_position(const glm::vec3& p) {
    std::lock_guard<std::mutex> lck(mut);
    position = p;
}

void Waveform::update(float dt) {
    std::lock_guard<std::mutex> lck(mut);
    while (!incoming_work_queue.empty()) {
        auto item = incoming_work_queue.pop();
        if (item) {
            (*item)();
        }
    }

    if (downsampled.size() > previous_size) {
        previous_size = downsampled.size();
        auto g = compute_geometry(downsampled);
        geometry.data(g);
        colors.data(compute_colours(g));
        ibo.data(compute_indices(g.size()));
    }
}

glm::mat4 Waveform::get_scale_matrix() const {
    return glm::scale(glm::vec3{time_scale, amplitude_scale, 1});
}

void Waveform::draw() const {
    std::lock_guard<std::mutex> lck(mut);
    auto s_shader = shader->get_scoped();
    shader->set_model_matrix(glm::translate(position) * get_scale_matrix());

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_TRIANGLE_STRIP, ibo.size(), GL_UNSIGNED_INT, nullptr);
}

//  inherited from GLAudioThumbnailBase
void Waveform::clear() {
    std::lock_guard<std::mutex> lck(mut);
    clear_impl();
}
void Waveform::load_from(AudioFormatManager& manager, const File& file) {
    std::lock_guard<std::mutex> lck(mut);
    load_from(
            std::unique_ptr<AudioFormatReader>(manager.createReaderFor(file)));
}

//  these two will be called from a thread *other* than the gl thread
void Waveform::reset(int num_channels,
                     double sample_rate,
                     int64 total_samples) {
    std::lock_guard<std::mutex> lck(mut);
    incoming_work_queue.push([this] { clear_impl(); });
}
void Waveform::addBlock(int64 sample_number_in_source,
                        const AudioSampleBuffer& new_data,
                        int start_offset,
                        int num_samples) {
    std::lock_guard<std::mutex> lck(mut);
    auto waveform_steps = num_samples / per_buffer;
    x_spacing = waveform_steps / load_context->sample_rate;
    std::vector<std::pair<float, float>> ret(per_buffer);
    for (auto a = 0u, b = 0u; a != per_buffer; ++a, b += waveform_steps) {
        auto mm = std::minmax_element(
                new_data.getReadPointer(0) + b,
                new_data.getReadPointer(0) + b + waveform_steps);
        ret[a] = std::make_pair(*mm.first, *mm.second);
    }

    incoming_work_queue.push([this, ret] {
        downsampled.insert(downsampled.end(), ret.begin(), ret.end());
    });
}

void Waveform::set_amplitude_scale(float f) {
    std::lock_guard<std::mutex> lck(mut);
    amplitude_scale = f;
}

void Waveform::set_time_scale(float f) {
    std::lock_guard<std::mutex> lck(mut);
    time_scale = f;
}

float Waveform::get_length_in_seconds() const {
    std::lock_guard<std::mutex> lck(mut);
    if (load_context) {
        return load_context->length_in_samples / load_context->sample_rate;
    }
    return 0;
}

void Waveform::load_from(std::unique_ptr<AudioFormatReader>&& reader) {
    load_context = std::make_unique<LoadContext>(*this, std::move(reader));
}

void Waveform::clear_impl() {
    previous_size = 0;
    downsampled.clear();
    geometry.clear();
    colors.clear();
    ibo.clear();
}

std::vector<glm::vec4> Waveform::compute_colours(
        const std::vector<glm::vec3>& g) {
    std::vector<glm::vec4> ret(g.size());
    proc::transform(g, ret.begin(), [](const auto& i) {
        return glm::mix(glm::vec4{0.4, 0.4, 0.4, 1},
                        glm::vec4{1, 1, 1, 1},
                        i.y * 0.5 + 0.5);
    });
    return ret;
}

std::vector<glm::vec3> Waveform::compute_geometry(
        const std::vector<std::pair<float, float>>& data) {
    std::vector<glm::vec3> ret;
    auto x = 0.0;
    for (const auto& i : data) {
        ret.push_back(glm::vec3{x, i.first, 0});
        ret.push_back(glm::vec3{x, i.second, 0});
        x += x_spacing;
    }
    return ret;
}
