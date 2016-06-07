#include "BasicDrawableObject.hpp"
#include "ImpulseRenderer.hpp"

#include "common/fftwf_helpers.h"
#include "common/sinc.h"
#include "common/stl_wrappers.h"

#include "modern_gl_utils/generic_shader.h"

#include <array>

namespace {

const char* test_files[] = {
    "/Users/reuben/dev/waveguide/demo/assets/sweep.wav",
    "/Users/reuben/dev/waveguide/demo/assets/noise.wav",
    "/Users/reuben/dev/pyverb/impulse.aiff",
};

//  might need an async solution if this is really slow...
auto load_test_file() {
    AudioFormatManager m;
    m.registerBasicFormats();
    std::unique_ptr<AudioFormatReader> reader(
        m.createReaderFor(File(test_files[2])));

    AudioSampleBuffer buffer(reader->numChannels, reader->lengthInSamples);
    reader->read(&buffer, 0, reader->lengthInSamples, 0, true, true);

    std::vector<std::vector<float>> ret(
        reader->numChannels, std::vector<float>(reader->lengthInSamples));
    for (auto i = 0; i != reader->numChannels; ++i) {
        std::copy(buffer.getReadPointer(i),
                  buffer.getReadPointer(i) + reader->lengthInSamples,
                  ret[i].begin());
    }
    return ret;
}

std::vector<GLuint> compute_indices(GLuint num) {
    std::vector<GLuint> ret(num);
    proc::iota(ret, 0);
    return ret;
}

const float samples_per_unit{88200};

//----------------------------------------------------------------------------//

class Waveform : public BasicDrawableObject {
public:
    Waveform(const GenericShader& shader, const std::vector<float>& signal)
            : Waveform(shader, compute_downsampled_waveform(signal)) {
    }

private:
    static const int waveform_steps = 256;
    static const float spacing;

    static std::vector<std::pair<float, float>> compute_downsampled_waveform(
        const std::vector<float>& i) {
        std::vector<std::pair<float, float>> ret(i.size() / waveform_steps);
        for (auto a = 0u, b = 0u; a < i.size() - waveform_steps;
             a += waveform_steps, ++b) {
            auto mm = std::minmax_element(i.begin() + a,
                                          i.begin() + a + waveform_steps);
            ret[b] = std::make_pair(*mm.first, *mm.second);
        }
        return ret;
    }

    static std::vector<std::vector<std::pair<float, float>>>
    compute_downsampled_waveform(const std::vector<std::vector<float>>& in) {
        std::vector<std::vector<std::pair<float, float>>> ret(in.size());
        proc::transform(in, ret.begin(), [](const auto& i) {
            return compute_downsampled_waveform(i);
        });
        return ret;
    }

    Waveform(const GenericShader& shader,
             const std::vector<std::pair<float, float>>& data)
            : Waveform(shader, compute_geometry(data)) {
    }

    Waveform(const GenericShader& shader, const std::vector<glm::vec3>& g)
            : BasicDrawableObject(shader,
                                  g,
                                  compute_colours(g),
                                  compute_indices(g.size()),
                                  GL_TRIANGLE_STRIP) {
    }

    static std::vector<glm::vec4> compute_colours(
        const std::vector<glm::vec3>& g) {
        std::vector<glm::vec4> ret(g.size());
        proc::transform(g, ret.begin(), [](const auto& i) {
            return glm::mix(glm::vec4{0.4, 0.4, 0.4, 1},
                            glm::vec4{1, 1, 1, 1},
                            i.y * 0.5 + 0.5);
        });
        return ret;
    }

    static std::vector<glm::vec3> compute_geometry(
        const std::vector<std::pair<float, float>>& data) {
        std::vector<glm::vec3> ret;
        auto x = 0.0;
        for (const auto& i : data) {
            ret.push_back(glm::vec3{x, i.first, 0});
            ret.push_back(glm::vec3{x, i.second, 0});
            x += spacing;
        }
        return ret;
    }
};

const float Waveform::spacing{waveform_steps / samples_per_unit};

//----------------------------------------------------------------------------//

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
        std::vector<std::vector<float>> ret;
        auto lim = input.size() - window_length;
        for (auto a = 0u; a < lim; a += hop_size) {
            ret.push_back(compute_slice(input.begin() + a,
                                        input.begin() + a + window_length));
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
                auto ret =
                    ((a[0] * a[0]) + (a[1] * a[1])) / normalisation_factor;
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

//----------------------------------------------------------------------------//

class Waterfall : public ::Drawable {
public:
    Waterfall(const GenericShader& shader, const std::vector<float>& signal)
            : Waterfall(shader,
                        Spectrogram(window, hop).compute(signal),
                        spacing,
                        2.0) {
    }

    void draw() const override {
        auto s_shader = shader.get_scoped();
        shader.set_model_matrix(glm::translate(glm::vec3{0, 0, 0}));
        for (const auto& i : strips) {
            i.draw();
        }
    }

    enum class Mode { linear, log };

    void set_mode(Mode u) {
        if (u != mode) {
            mode = u;
            strips =
                compute_strips(shader, spectrogram, mode, x_spacing, z_width);
        }
    };

private:
    Waterfall(const GenericShader& shader,
              const std::vector<std::vector<float>>& heights,
              float x_spacing,
              float z_width)
            : shader(shader)
            , spectrogram(heights)
            , x_spacing(x_spacing)
            , z_width(z_width)
            , strips(compute_strips(
                  shader, spectrogram, mode, x_spacing, z_width)) {
    }

    class HeightMapStrip : public ::Drawable {
    public:
        HeightMapStrip(const GenericShader& shader,
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

        void draw() const override {
            auto s_vao = vao.get_scoped();
            glDrawElements(GL_TRIANGLE_STRIP, size, GL_UNSIGNED_INT, nullptr);
        }

    private:
        static std::vector<glm::vec3> compute_geometry(
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
                        ret[j + 1] =
                            glm::vec3{x + x_spacing, right[i], z_spacing * i};
                    }
                    break;
                }
                case Mode::log: {
                    auto min_frequency = 0.001;
                    auto min_log = std::log(min_frequency);
                    for (auto i = 0u, j = 0u; i != count; ++i, j += 2) {
                        auto bin_frequency = i / static_cast<float>(count);
                        auto z_pos =
                            bin_frequency < min_frequency
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

        static glm::vec3 compute_mapped_colour(float r) {
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

        static std::vector<glm::vec4> compute_colors(
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

        const GenericShader& shader;

        VAO vao;
        StaticVBO geometry;
        StaticVBO colors;
        StaticIBO ibo;
        GLuint size;
    };

    static std::vector<HeightMapStrip> compute_strips(
        const GenericShader& generic_shader,
        const std::vector<std::vector<float>>& input,
        Mode mode,
        float x_spacing,
        float z_width) {
        std::vector<HeightMapStrip> ret;
        auto x = 0.0;
        std::transform(
            input.begin(),
            input.end() - 1,
            input.begin() + 1,
            std::back_inserter(ret),
            [&generic_shader, &x, mode, x_spacing, z_width](const auto& i,
                                                            const auto& j) {
                HeightMapStrip ret(
                    generic_shader, i, j, mode, x, x_spacing, z_width);
                x += x_spacing;
                return ret;
            });
        return ret;
    }

private:
    static const int window{1024};
    static const int hop{1024};
    static const float spacing;

    const GenericShader& shader;

    Mode mode{Mode::log};

    std::vector<std::vector<float>> spectrogram;

    float x_spacing;
    float z_width;

    std::vector<HeightMapStrip> strips;
};

const float Waterfall::spacing{hop / samples_per_unit};

}  // namespace

//----------------------------------------------------------------------------//

class ImpulseRenderer::ContextLifetime : public BaseContextLifetime {
public:
    //  TODO remove
    ContextLifetime()
            : ContextLifetime(load_test_file()) {
    }

    ContextLifetime(const std::vector<std::vector<float>>& audio)
            : waveform(generic_shader, audio.front())
            , waterfall(generic_shader, audio.front()) {
        waveform.set_position(glm::vec3{0, 0, 2});
    }

    void update(float dt) override {
        current_azel += (target_azel - current_azel) * 0.1;
    }

    void draw() const override {
        auto c = 0.0;
        glClearColor(c, c, c, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POLYGON_SMOOTH);
        /*
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        */

        auto config_shader = [this](const auto& shader) {
            auto s_shader = shader.get_scoped();
            shader.set_model_matrix(glm::mat4());
            shader.set_view_matrix(get_view_matrix());
            shader.set_projection_matrix(get_projection_matrix());
        };

        config_shader(generic_shader);

        waveform.draw();
        waterfall.draw();
    }

    void set_mode(Mode u) {
        mode = u;

        switch (mode) {
            case Mode::waveform:
                target_azel = waveform_azel;
                break;
            case Mode::waterfall:
                target_azel = waterfall_azel;
                break;
        }
    }

    void mouse_down(const glm::vec2& pos) override {
        mousing = std::make_unique<Rotate>(target_azel, pos);
    }

    void mouse_drag(const glm::vec2& pos) override {
        assert(mousing);

        const auto& m = dynamic_cast<Rotate&>(*mousing);
        auto diff = pos - m.position;
        target_azel = Orientable::AzEl{
            m.orientation.azimuth + diff.x * Rotate::angle_scale,
            glm::clamp(m.orientation.elevation + diff.y * Rotate::angle_scale,
                       static_cast<float>(-M_PI / 2),
                       static_cast<float>(M_PI / 2))};
    }

    void mouse_up(const glm::vec2& pos) override {
        mousing = nullptr;
    }

    void mouse_wheel_move(float delta_y) override {
    }

private:
    glm::mat4 get_projection_matrix() const {
        return glm::perspective(45.0f, get_aspect(), 0.05f, 1000.0f);
    }

    glm::mat4 get_rotation_matrix() const {
        auto i = glm::rotate(current_azel.azimuth, glm::vec3(0, 1, 0));
        auto j = glm::rotate(current_azel.elevation, glm::vec3(1, 0, 0));
        return j * i;
    }

    glm::mat4 get_view_matrix() const {
        return glm::lookAt(
                   glm::vec3{0, 0, eye}, glm::vec3{0}, glm::vec3{0, 1, 0}) *
               get_rotation_matrix();
    }

    struct Mousing {
        virtual ~Mousing() noexcept = default;
        enum class Mode { rotate, move };
        virtual Mode get_mode() const = 0;
    };

    struct Rotate : public Mousing {
        Rotate(const Orientable::AzEl& azel, const glm::vec2& position)
                : orientation(azel)
                , position(position) {
        }
        Mode get_mode() const {
            return Mode::rotate;
        }

        static const float angle_scale;
        Orientable::AzEl orientation;
        glm::vec2 position;
    };

    GenericShader generic_shader;

    static const Orientable::AzEl waveform_azel;
    static const Orientable::AzEl waterfall_azel;
    static const float angle_scale;

    Orientable::AzEl current_azel{0, 0};
    Orientable::AzEl target_azel{0, 0};
    float eye{4};

    Mode mode;
    Waveform waveform;
    Waterfall waterfall;

    std::unique_ptr<Mousing> mousing;
};

const Orientable::AzEl ImpulseRenderer::ContextLifetime::waveform_azel{0, 0};
const Orientable::AzEl ImpulseRenderer::ContextLifetime::waterfall_azel{-0.7,
                                                                        0.2};

const float ImpulseRenderer::ContextLifetime::Rotate::angle_scale{0.01};

//----------------------------------------------------------------------------//

ImpulseRenderer::ImpulseRenderer() {
}

ImpulseRenderer::~ImpulseRenderer() noexcept = default;

void ImpulseRenderer::newOpenGLContextCreated() {
    std::lock_guard<std::mutex> lck(mut);
    context_lifetime = std::make_unique<ContextLifetime>();
    BaseRenderer::newOpenGLContextCreated();
}

void ImpulseRenderer::openGLContextClosing() {
    std::lock_guard<std::mutex> lck(mut);
    context_lifetime = nullptr;
    BaseRenderer::openGLContextClosing();
}

BaseContextLifetime* ImpulseRenderer::get_context_lifetime() {
    return context_lifetime.get();
}

void ImpulseRenderer::set_mode(Mode mode) {
    std::lock_guard<std::mutex> lck(mut);
    push_incoming([this, mode] { context_lifetime->set_mode(mode); });
}

//----------------------------------------------------------------------------//

ImpulseRendererComponent::ImpulseRendererComponent() {
}

void ImpulseRendererComponent::set_mode(ImpulseRenderer::Mode mode) {
    get_renderer().set_mode(mode);
}