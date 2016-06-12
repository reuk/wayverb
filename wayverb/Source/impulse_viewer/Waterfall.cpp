#include "Waterfall.hpp"

#include "ComputeIndices.hpp"

#include "common/fftwf_helpers.h"
#include "common/sinc.h"

#include "glm/gtx/transform.hpp"

#include <array>
#include <iomanip>
#include <sstream>

namespace {
class Spectrogram {
public:
    Spectrogram(int window_length, int hop_size)
            : window_length(window_length)
            , complex_length(window_length / 2 + 1)
            , hop_size(hop_size)
            , window(blackman(window_length))
            , i(fftwf_alloc_real(window_length))
            , o(fftwf_alloc_complex(complex_length))
            , r2c(fftwf_plan_dft_r2c_1d(
                      window_length, i.get(), o.get(), FFTW_ESTIMATE)) {
        std::vector<float> ones(window_length, 1);
        normalisation_factor = 1;
        normalisation_factor = Decibels::decibelsToGain(
                compute_slice(ones.begin(), ones.end()).front());
    }
    virtual ~Spectrogram() noexcept = default;

    std::vector<std::vector<float>> compute(const std::vector<float>& input) {
        return compute(input.begin(), input.end());
    }

    template <typename It>
    std::vector<std::vector<float>> compute(It begin, It end) {
        std::vector<std::vector<float>> ret;
        auto lim = std::distance(begin, end) - window_length + 1;
        for (auto a = 0u; a < lim; a += hop_size) {
            ret.push_back(compute_slice(begin + a, begin + a + window_length));
        }
        return ret;
    }

private:
    template <typename It>
    std::vector<float> compute_slice(It begin, It end) {
        assert(std::distance(begin, end) == window_length);
        std::transform(begin, end, window.begin(), i.get(), [](auto a, auto b) {
            return a * b;
        });
        fftwf_execute(r2c);
        std::vector<float> ret(complex_length);
        std::transform(
                o.get(), o.get() + complex_length, ret.begin(), [this](auto a) {
                    auto ret = ((a[0] * a[0]) + (a[1] * a[1])) /
                               normalisation_factor;
                    //  get decibel value
                    ret = Decibels::gainToDecibels(ret);
                    return ret;
                });
        return ret;
    }

    int window_length;
    int complex_length;
    int hop_size;

    std::vector<float> window;
    float normalisation_factor;

    fftwf_r i;
    fftwf_c o;
    FftwfPlan r2c;
};

}  // namespace

//----------------------------------------------------------------------------//

Waterfall::Waterfall(MatrixTreeNode* parent,
                     WaterfallShader& waterfall_shader,
                     FadeShader& fade_shader,
                     TexturedQuadShader& quad_shader)
        : MatrixTreeNode(parent)
        , waterfall_shader(&waterfall_shader)
        , fade_shader(&fade_shader)
        , quad_shader(&quad_shader) {
}

void Waterfall::set_position(const glm::vec3& p) {
    std::lock_guard<std::mutex> lck(mut);
    position = p;
}

void Waterfall::update(float dt) {
    std::lock_guard<std::mutex> lck(mut);
    while (!incoming_work_queue.empty()) {
        auto item = incoming_work_queue.pop();
        if (item) {
            (*item)();
        }
    }

    auto begin = std::chrono::system_clock::now();
    while (strips.size() + 1 < spectrum.size() &&
           (std::chrono::system_clock::now() - begin) <
                   std::chrono::duration<double>(1 / 200.0)) {
        strips.emplace_back(this,
                            *waterfall_shader,
                            spectrum[strips.size()],
                            spectrum[strips.size() + 1],
                            mode,
                            x_spacing * strips.size(),
                            x_spacing,
                            min_frequency,
                            max_frequency,
                            load_context->sample_rate);
    }
}

glm::vec3 Waterfall::get_scale() const {
    return glm::vec3{1, 1, width};
}

void Waterfall::draw() const {
    std::lock_guard<std::mutex> lck(mut);

    assert(glGetError() == GL_NO_ERROR);

    {
        auto s = waterfall_shader->get_scoped();
        waterfall_shader->set_model_matrix(get_modelview_matrix());
        for (const auto& i : strips) {
            i.draw();
        }
    }

    AxisObject axis(this, *fade_shader, *quad_shader);
    auto seconds = load_context->length_in_samples / load_context->sample_rate;

    {
        // frequency marks

        axis.set_scale(glm::vec3{seconds, 1, 1});
        for (auto i = 0; i != axes; ++i) {
            auto z = i / (axes - 1.0);
            std::stringstream ss;
            ss << std::setprecision(2) << z_to_frequency(mode, z) / 1000;
            axis.set_label(ss.str());
            axis.set_position(glm::vec3{0, 0.01, z});
            axis.draw();
        }
    }

    {
        //  time marks

        axis.set_scale(glm::vec3{1, 1, 1});
        axis.set_azimuth(-M_PI / 2);

        for (auto i = 1; i <= std::floor(seconds); ++i) {
            auto time = i;
            std::stringstream ss;
            ss << time;
            axis.set_label(ss.str());
            axis.set_position(glm::vec3{i, 0.01, 0});
            axis.draw();
        }
    }
}

void Waterfall::set_mode(Mode u) {
    std::lock_guard<std::mutex> lck(mut);
    if (u != mode) {
        mode = u;

        strips.clear();

        auto x = 0.0;
        std::transform(spectrum.begin(),
                       spectrum.end() - 1,
                       spectrum.begin() + 1,
                       std::back_inserter(strips),
                       [this, &x](const auto& i, const auto& j) {
                           HeightMapStrip ret(this,
                                              *waterfall_shader,
                                              i,
                                              j,
                                              mode,
                                              x,
                                              x_spacing,
                                              min_frequency,
                                              max_frequency,
                                              load_context->sample_rate);
                           x += x_spacing;
                           return ret;
                       });
    }
}

//  inherited from GLAudioThumbnailBase
void Waterfall::clear() {
    std::lock_guard<std::mutex> lck(mut);
    clear_impl();
}
void Waterfall::load_from(AudioFormatManager& manager, const File& file) {
    std::lock_guard<std::mutex> lck(mut);
    load_from(
            std::unique_ptr<AudioFormatReader>(manager.createReaderFor(file)));
}

//  these two will be called from a thread *other* than the gl thread
void Waterfall::reset(int num_channels,
                      double sample_rate,
                      int64 total_samples) {
    std::lock_guard<std::mutex> lck(mut);
    incoming_work_queue.push([this] { clear_impl(); });
}
void Waterfall::addBlock(int64 sample_number_in_source,
                         const AudioSampleBuffer& new_data,
                         int start_offset,
                         int num_samples) {
    std::lock_guard<std::mutex> lck(mut);
    auto l = num_samples / per_buffer;

    x_spacing = l / load_context->sample_rate;

    auto ptr = new_data.getReadPointer(0);
    Spectrogram spec(l, l);
    auto s = spec.compute(ptr, ptr + num_samples);
    for (auto& i : s) {
        for (auto& j : i) {
            //  to take the value back into the 0-1 range
            j = (j + 100) / 100;
        }
    }
    incoming_work_queue.push(
            [this, s] { spectrum.insert(spectrum.end(), s.begin(), s.end()); });
}

void Waterfall::load_from(std::unique_ptr<AudioFormatReader>&& reader) {
    load_context = std::make_unique<LoadContext>(*this, std::move(reader));
}

void Waterfall::clear_impl() {
    strips.clear();
    spectrum.clear();
}

float Waterfall::z_to_frequency(Mode mode, float z) {
    switch (mode) {
        case Mode::linear: {
            return z * (max_frequency - min_frequency) + min_frequency;
        }
        case Mode::log: {
            auto min_log = std::log10(min_frequency);
            auto max_log = std::log10(max_frequency);
            return std::pow(10, z * (max_log - min_log) + min_log);
        }
    }
}

float Waterfall::frequency_to_z(Mode mode, float frequency) {
    switch (mode) {
        case Mode::linear: {
            return (frequency - min_frequency) /
                   (max_frequency - min_frequency);
        }
        case Mode::log: {
            auto min_log = std::log10(min_frequency);
            auto max_log = std::log10(max_frequency);
            auto bin_log = std::log10(frequency);
            return (bin_log - min_log) / (max_log - min_log);
        }
    }
}

glm::mat4 Waterfall::get_local_modelview_matrix() const {
    return glm::translate(position) * glm::scale(get_scale());
}

//----------------------------------------------------------------------------//

Waterfall::HeightMapStrip::HeightMapStrip(MatrixTreeNode* parent,
                                          WaterfallShader& shader,
                                          const std::vector<float>& left,
                                          const std::vector<float>& right,
                                          Mode mode,
                                          float x,
                                          float x_spacing,
                                          float min_frequency,
                                          float max_frequency,
                                          float sample_rate)
        : MatrixTreeNode(parent)
        , shader(&shader)
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
    ibo.data(compute_indices(g.size()));

    auto s = vao.get_scoped();

    geometry.bind();
    auto v_pos = shader.get_attrib_location("v_position");
    glEnableVertexAttribArray(v_pos);
    glVertexAttribPointer(v_pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    ibo.bind();
}

glm::mat4 Waterfall::HeightMapStrip::get_local_modelview_matrix() const {
    return glm::mat4{};
}

void Waterfall::HeightMapStrip::draw() const {
    auto s_shader = shader->get_scoped();
    shader->set_model_matrix(get_modelview_matrix());

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_TRIANGLE_STRIP, ibo.size(), GL_UNSIGNED_INT, nullptr);
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
