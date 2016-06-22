#include "Waterfall.hpp"

#include "for_each.h"
#include "lerp.h"
#include "modern_gl_utils/geometry_helpers.h"

#include "glm/gtx/transform.hpp"

#include <array>
#include <iomanip>
#include <sstream>

Waterfall::Waterfall(WaterfallShader& waterfall_shader,
                     FadeShader& fade_shader,
                     TexturedQuadShader& quad_shader,
                     AudioFormatManager& manager,
                     const File& file)
        : waterfall_shader(&waterfall_shader)
        , fade_shader(&fade_shader)
        , quad_shader(&quad_shader)
        , loader(std::unique_ptr<AudioFormatReader>(
                         manager.createReaderFor(file)),
                 1 << 13,
                 1 << 10,
                 1 << 9) {
    auto seconds = loader.get_length_in_seconds();
    for (auto i = 0; i != axes; ++i) {
        auto z = i / (axes - 1.0);
        std::stringstream ss;
        ss << std::setprecision(3);
        auto f = z_to_frequency(mode, z);
        if (1000 <= f) {
            ss << f / 1000 << "K";
        } else {
            ss << f;
        }
        auto axis = std::make_unique<AxisObject>(
                fade_shader, quad_shader, ss.str());
        axis->set_scale(glm::vec3{seconds, 1, 1});
        axis->set_position(glm::vec3{0, 0.01, z});
        frequency_axis_objects.push_back(std::move(axis));
    }
}

void Waterfall::set_position(const glm::vec3& p) {
    std::lock_guard<std::mutex> lck(mut);
    position = p;
}

void Waterfall::update(float dt) {
    std::lock_guard<std::mutex> lck(mut);
    auto begin = std::chrono::system_clock::now();
    auto spectrum = loader.get_channel(channel);
    while (strips.size() + 1 < spectrum.size() &&
           (std::chrono::system_clock::now() - begin) <
                   std::chrono::duration<double>(1 / 200.0)) {
        strips.emplace_back(*waterfall_shader,
                            spectrum[strips.size()],
                            spectrum[strips.size() + 1],
                            mode,
                            loader.get_x_spacing() * strips.size(),
                            loader.get_x_spacing(),
                            min_frequency,
                            max_frequency,
                            loader.get_sample_rate());
    }

    auto seconds = loader.get_length_in_seconds();
    auto padding = visible_range.getLength() * 2;
    auto start = std::floor(std::max(0.0, visible_range.getStart() - padding) /
                            time_axis_interval) *
                         time_axis_interval +
                 time_axis_interval;
    time_axis_objects.clear();
    for (auto i = start;
         i <= std::min(seconds, visible_range.getEnd() + padding);
         i += time_axis_interval) {
        std::stringstream ss;
        ss << i;
        auto axis = std::make_unique<AxisObject>(
                *fade_shader, *quad_shader, ss.str());
        axis->set_position(glm::vec3{i, 0.01, 1});
        axis->set_scale(glm::vec3{1, 1, 1});
        axis->set_azimuth(M_PI / 2);
        time_axis_objects.push_back(std::move(axis));
    }
}

void Waterfall::set_channel(size_t c) {
    std::lock_guard<std::mutex> lck(mut);
    channel = c;
    strips.clear();
}

glm::vec3 Waterfall::get_scale() const {
    return glm::vec3{1, 1, width};
}

void Waterfall::do_draw(const glm::mat4& modelview_matrix) const {
    std::lock_guard<std::mutex> lck(mut);

    assert(glGetError() == GL_NO_ERROR);

    for (const auto& i : strips) {
        i.draw(modelview_matrix);
    }

    for (const auto& i : frequency_axis_objects) {
        i->draw(modelview_matrix);
    }

    for (const auto& i : time_axis_objects) {
        i->draw(modelview_matrix);
    }
}

glm::mat4 Waterfall::get_local_modelview_matrix() const {
    return glm::translate(position) * glm::scale(get_scale());
}

void Waterfall::set_mode(Mode u) {
    std::lock_guard<std::mutex> lck(mut);
    if (u != mode) {
        mode = u;

        strips.clear();
        auto spectrum = loader.get_channel(channel);

        auto x = 0.0;
        std::transform(spectrum.begin(),
                       spectrum.end() - 1,
                       spectrum.begin() + 1,
                       std::back_inserter(strips),
                       [this, &x](const auto& i, const auto& j) {
                           HeightMapStrip ret(*waterfall_shader,
                                              i,
                                              j,
                                              mode,
                                              x,
                                              loader.get_x_spacing(),
                                              min_frequency,
                                              max_frequency,
                                              loader.get_sample_rate());
                           x += loader.get_x_spacing();
                           return ret;
                       });
    }
}

void Waterfall::set_visible_range(const Range<double>& r) {
    std::lock_guard<std::mutex> lck(mut);
    visible_range = r;
    time_axis_interval = std::pow(
            10, std::ceil(std::log10(visible_range.getLength() * 0.2)));
}

float Waterfall::z_to_frequency(Mode mode, float z) {
    switch (mode) {
        case Mode::linear: {
            return lerp(z, 0.0f, 1.0f, max_frequency, min_frequency);
        }
        case Mode::log: {
            return std::pow(10,
                            lerp(z,
                                 0.0f,
                                 1.0f,
                                 std::log10(max_frequency),
                                 std::log10(min_frequency)));
        }
    }
}

float Waterfall::frequency_to_z(Mode mode, float frequency) {
    switch (mode) {
        case Mode::linear: {
            return lerp(frequency, max_frequency, min_frequency, 0.0f, 1.0f);
        }
        case Mode::log: {
            return lerp(std::log10(frequency),
                        std::log10(max_frequency),
                        std::log10(min_frequency),
                        0.0f,
                        1.0f);
        }
    }
}

//----------------------------------------------------------------------------//

Waterfall::HeightMapStrip::HeightMapStrip(WaterfallShader& shader,
                                          const std::vector<float>& left,
                                          const std::vector<float>& right,
                                          Mode mode,
                                          float x,
                                          float x_spacing,
                                          float min_frequency,
                                          float max_frequency,
                                          float sample_rate)
        : shader(&shader)
        , size(left.size() * 2) {
    auto g = compute_geometry(left,
                              right,
                              mode,
                              x,
                              x_spacing,
                              min_frequency,
                              max_frequency,
                              sample_rate);

    geometry.data(g);
    ibo.data(mglu::compute_indices<GLuint>(g.size()));

    auto s = vao.get_scoped();

    geometry.bind();
    auto v_pos = shader.get_attrib_location("v_position");
    glEnableVertexAttribArray(v_pos);
    glVertexAttribPointer(v_pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    ibo.bind();
}

void Waterfall::HeightMapStrip::do_draw(
        const glm::mat4& modelview_matrix) const {
    auto s_shader = shader->get_scoped();
    shader->set_model_matrix(modelview_matrix);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_TRIANGLE_STRIP, ibo.size(), GL_UNSIGNED_INT, nullptr);
}

glm::mat4 Waterfall::HeightMapStrip::get_local_modelview_matrix() const {
    return glm::mat4{};
}

std::vector<glm::vec3> Waterfall::HeightMapStrip::compute_geometry(
        const std::vector<float>& left,
        const std::vector<float>& right,
        Mode mode,
        float x,
        float x_spacing,
        float min_frequency,
        float max_frequency,
        float sample_rate) {
    assert(left.size() == right.size());
    auto count = left.size();

    int min_bin = std::ceil(count * min_frequency / (sample_rate * 0.5));
    int max_bin = std::floor(count * max_frequency / (sample_rate * 0.5));

    std::vector<glm::vec3> ret((max_bin - min_bin) * 2);

    for (auto i = min_bin, j = 0; i != max_bin; ++i, j += 2) {
        auto bin_frequency = i * sample_rate * 0.5 / count;
        auto z_pos = frequency_to_z(mode, bin_frequency);
        ret[j + 0] = glm::vec3{x, left[i], z_pos};
        ret[j + 1] = glm::vec3{x + x_spacing, right[i], z_pos};
    }

    return ret;
}

const float Waterfall::min_frequency{20};
const float Waterfall::max_frequency{20000};

const float Waterfall::width{3};
