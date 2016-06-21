#include "Waveform.hpp"

#include "modern_gl_utils/geometry_helpers.h"

#include "glm/gtx/transform.hpp"

Waveform::Waveform(mglu::GenericShader& shader,
                   AudioFormatManager& manager,
                   const File& file)
        : shader(&shader)
        , loader(std::unique_ptr<AudioFormatReader>(
                         manager.createReaderFor(file)),
                 1 << 13,
                 1 << 8,
                 1 << 8) {
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
    auto downsampled = loader.get_channel(channel);
    if (downsampled.size() > previous_size) {
        previous_size = downsampled.size();
        auto g = compute_geometry(downsampled);
        geometry.data(g);
        colors.data(compute_colours(g));
        ibo.data(mglu::compute_indices<GLuint>(g.size()));
    }
}

void Waveform::do_draw(const glm::mat4& modelview_matrix) const {
    std::lock_guard<std::mutex> lck(mut);
    auto s_shader = shader->get_scoped();
    shader->set_model_matrix(modelview_matrix);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_TRIANGLE_STRIP, ibo.size(), GL_UNSIGNED_INT, nullptr);
}

glm::mat4 Waveform::get_local_modelview_matrix() const {
    return glm::translate(position);
}

std::vector<glm::vec4> Waveform::compute_colours(
        const std::vector<glm::vec3>& g) {
    std::vector<glm::vec4> ret(g.size());
    std::transform(g.begin(), g.end(), ret.begin(), [](const auto& i) {
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
        x += loader.get_x_spacing();
    }
    return ret;
}
