#include "Waterfall.hpp"

#include "ComputeIndices.hpp"

#include "common/fftwf_helpers.h"
#include "common/sinc.h"

#include "glm/gtx/transform.hpp"

#include <array>

namespace {
class Spectrogram {
public:
    Spectrogram(int window_length, int hop_size)
            : window_length(window_length)
            , complex_length(window_length / 2 + 1)
            , hop_size(hop_size)
            , window(blackman(window_length))
            , normalisation_factor(compute_normalization_factor(window))
            , i(fftwf_alloc_real(window_length))
            , o(fftwf_alloc_complex(complex_length))
            , r2c(fftwf_plan_dft_r2c_1d(
                      window_length, i.get(), o.get(), FFTW_ESTIMATE)) {
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
                    //  to take the value back into the 0-1 range
                    ret = (ret + 100) / 100;
                    return ret;
                });
        return ret;
    }

    static float compute_normalization_factor(
            const std::vector<float>& window) {
        return proc::accumulate(window, 0.0);
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

Waterfall::Waterfall(const FadeShader& shader)
        : shader(shader) {
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
        strips.emplace_back(shader,
                            spectrum[strips.size()],
                            spectrum[strips.size() + 1],
                            mode,
                            x_spacing * strips.size(),
                            x_spacing,
                            z_width);
    }
}

void Waterfall::draw() const {
    std::lock_guard<std::mutex> lck(mut);
    auto s_shader = shader.get_scoped();
    shader.set_model_matrix(glm::translate(position));
    for (const auto& i : strips) {
        i.draw();
    }
}

void Waterfall::set_mode(Mode u) {
    std::lock_guard<std::mutex> lck(mut);
    if (u != mode) {
        mode = u;
        strips = compute_strips(shader, spectrum, mode, x_spacing, z_width);
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

    auto s = Spectrogram(l, l).compute(ptr, ptr + num_samples);
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

std::vector<Waterfall::HeightMapStrip> Waterfall::compute_strips(
        const FadeShader& shader,
        const std::vector<std::vector<float>>& input,
        Mode mode,
        float x_spacing,
        float z_width) {
    std::vector<HeightMapStrip> ret;
    auto x = 0.0;
    std::transform(input.begin(),
                   input.end() - 1,
                   input.begin() + 1,
                   std::back_inserter(ret),
                   [&shader, &x, mode, x_spacing, z_width](const auto& i,
                                                           const auto& j) {
                       HeightMapStrip ret(
                               shader, i, j, mode, x, x_spacing, z_width);
                       x += x_spacing;
                       return ret;
                   });
    return ret;
}

//----------------------------------------------------------------------------//

Waterfall::HeightMapStrip::HeightMapStrip(const FadeShader& shader,
                                          const std::vector<float>& left,
                                          const std::vector<float>& right,
                                          Mode mode,
                                          float x,
                                          float x_spacing,
                                          float z_width)
        : shader(shader)
        , size(left.size() * 2) {
    auto g = compute_geometry(left, right, mode, x, x_spacing, z_width);
    assert(g.size() == size);

    geometry.data(g);
    colors.data(compute_colors(g));
    ibo.data(compute_indices(g.size()));

    auto s = vao.get_scoped();

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

void Waterfall::HeightMapStrip::draw() const {
    auto s_vao = vao.get_scoped();
    glDrawElements(GL_TRIANGLE_STRIP, size, GL_UNSIGNED_INT, nullptr);
}

std::vector<glm::vec3> Waterfall::HeightMapStrip::compute_geometry(
        const std::vector<float>& left,
        const std::vector<float>& right,
        Mode mode,
        float x,
        float x_spacing,
        float z_width) {
    assert(left.size() == right.size());
    auto count = left.size();

    std::vector<glm::vec3> ret(count * 2);

    switch (mode) {
        case Mode::linear: {
            auto z_spacing = z_width / count;
            for (auto i = 0u, j = 0u; i != count; ++i, j += 2) {
                ret[j + 0] = glm::vec3{x, left[i], z_spacing * i};
                ret[j + 1] = glm::vec3{x + x_spacing, right[i], z_spacing * i};
            }
            break;
        }
        case Mode::log: {
            auto min_frequency = 0.001;
            auto min_log = std::log(min_frequency);
            for (auto i = 0u, j = 0u; i != count; ++i, j += 2) {
                auto bin_frequency = i / static_cast<float>(count);
                auto z_pos = bin_frequency < min_frequency
                                     ? 0
                                     : -(std::log(bin_frequency) - min_log) *
                                               z_width / min_log;
                ret[j + 0] = glm::vec3{x, left[i], z_pos};
                ret[j + 1] = glm::vec3{x + x_spacing, right[i], z_pos};
            }
            break;
        }
    }
    return ret;
}

glm::vec3 Waterfall::HeightMapStrip::compute_mapped_colour(float r) {
    auto min = 0.2;
    auto max = 1;
    std::array<glm::vec3, 4> colours{glm::vec3{min, min, min},
                                     glm::vec3{min, max, min},
                                     glm::vec3{min, max, max},
                                     glm::vec3{max, max, max}};
    const auto d = 1.0 / (colours.size() - 1);
    for (auto i = 0u; i != colours.size() - 1; ++i, r -= d) {
        if (r < d) {
            return glm::mix(colours[i], colours[i + 1], r / d);
        }
    }
    return colours[0];
}

std::vector<glm::vec4> Waterfall::HeightMapStrip::compute_colors(
        const std::vector<glm::vec3>& g) {
    auto x = std::fmod(g.front().x, 1);
    auto c = compute_mapped_colour(x);
    std::vector<glm::vec4> ret(g.size());
    proc::transform(g, ret.begin(), [c](const auto& i) {
        return glm::mix(glm::vec4{0.0, 0.0, 0.0, 1},
                        glm::vec4{c, 1},
                        std::pow(i.y, 0.1));
    });
    return ret;
}
