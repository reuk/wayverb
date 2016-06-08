#include "BasicDrawableObject.hpp"
#include "FadeShader.hpp"
#include "ImpulseRenderer.hpp"

#include "common/fftwf_helpers.h"
#include "common/sinc.h"
#include "common/stl_wrappers.h"

#include "modern_gl_utils/generic_shader.h"

#include <array>
#include <thread>

//  what's the deal?

//  need to load audio file from disk and visualise it

//      each visualiser should open the file individually
//      fire of a worker thread of some sort to read the file in blocks
//      when each block finishes, updae the render

namespace {
std::vector<GLuint> compute_indices(GLuint num) {
    std::vector<GLuint> ret(num);
    proc::iota(ret, 0);
    return ret;
}

const float samples_per_unit{88200};

//----------------------------------------------------------------------------//

class LoadContext {
public:
    LoadContext(GLAudioThumbnailBase& o,
                std::unique_ptr<AudioFormatReader>&& m,
                int buffer_size = 4096)
            : owner(o)
            , audio_format_reader(std::move(m))
            , channels(audio_format_reader->numChannels)
            , length_in_samples(audio_format_reader->lengthInSamples)
            , sample_rate(audio_format_reader->sampleRate)
            , thread([this, buffer_size] {
                AudioSampleBuffer buffer(channels, buffer_size);
                for (; !is_fully_loaded() && keep_reading;
                     samples_read += buffer_size) {
                    audio_format_reader->read(
                        &buffer, 0, buffer_size, samples_read, true, true);
                    owner.addBlock(samples_read, buffer, 0, buffer_size);
                }
            }) {
    }

    bool is_fully_loaded() const {
        return samples_read >= length_in_samples;
    }

    virtual ~LoadContext() noexcept {
        keep_reading = false;
        thread.join();
    }

private:
    GLAudioThumbnailBase& owner;
    std::unique_ptr<AudioFormatReader> audio_format_reader;
    int samples_read{0};

public:
    const int channels;
    const int length_in_samples;
    const double sample_rate;

private:
    std::atomic_bool keep_reading{true};
    std::thread thread;
};

//----------------------------------------------------------------------------//

class Waveform : public ::Drawable, public GLAudioThumbnailBase {
public:
    /*
        Waveform(const GenericShader& shader, const std::vector<float>& signal)
                : Waveform(shader, compute_downsampled_waveform(signal)) {
        }
    */

    Waveform(const GenericShader& shader)
            : shader(shader) {
    }

    void set_position(const glm::vec3& p) {
        position = p;
    }

    void draw() const override {
    }

    //  inherited from GLAudioThumbnailBase
    void clear() override {
    }
    void load_from(AudioFormatManager& manager, const File& file) override {
        load_from(
            std::unique_ptr<AudioFormatReader>(manager.createReaderFor(file)));
    }
    void reset(int num_channels,
               double sample_rate,
               int64 total_samples) override {
    }
    void addBlock(int64 sample_number_in_source,
                  const AudioSampleBuffer& new_data,
                  int start_offset,
                  int num_samples) override {
    }

    void load_from(std::unique_ptr<AudioFormatReader>&& reader) {
        load_context = std::make_unique<LoadContext>(*this, std::move(reader));
    }

private:
    static const int waveform_steps = 256;
    static const float spacing;

    /*
        static std::vector<std::pair<float, float>>
       compute_downsampled_waveform(
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
        compute_downsampled_waveform(const std::vector<std::vector<float>>& in)
       {
            std::vector<std::vector<std::pair<float, float>>> ret(in.size());
            proc::transform(in, ret.begin(), [](const auto& i) {
                return compute_downsampled_waveform(i);
            });
            return ret;
        }
    */

    /*
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
    */

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

    const GenericShader& shader;

    std::unique_ptr<LoadContext> load_context;

    glm::vec3 position{0};
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

class Waterfall : public ::Drawable, public GLAudioThumbnailBase {
public:
    Waterfall(const FadeShader& shader)
            : shader(shader) {
    }

    /*
        Waterfall(const FadeShader& shader, const std::vector<float>& signal)
                : Waterfall(shader,
                            Spectrogram(window, hop).compute(signal),
                            spacing,
                            2.0) {
        }
    */

    void draw() const override {
        auto s_shader = shader.get_scoped();
        shader.set_model_matrix(glm::translate(position));
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

    void set_position(const glm::vec3& p) {
        position = p;
    }

    void clear() override {
    }
    void load_from(AudioFormatManager& manager, const File& file) override {
        load_from(
            std::unique_ptr<AudioFormatReader>(manager.createReaderFor(file)));
    }
    void reset(int num_channels,
               double sample_rate,
               int64 total_samples) override {
    }
    void addBlock(int64 sample_number_in_source,
                  const AudioSampleBuffer& new_data,
                  int start_offset,
                  int num_samples) override {
    }

    void load_from(std::unique_ptr<AudioFormatReader>&& reader) {
        load_context = std::make_unique<LoadContext>(*this, std::move(reader));
    }

private:
    /*
        Waterfall(const FadeShader& shader,
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
    */

    class HeightMapStrip : public ::Drawable {
    public:
        HeightMapStrip(const FadeShader& shader,
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

        const FadeShader& shader;

        VAO vao;
        StaticVBO geometry;
        StaticVBO colors;
        StaticIBO ibo;
        GLuint size;
    };

    static std::vector<HeightMapStrip> compute_strips(
        const FadeShader& generic_shader,
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

    const FadeShader& shader;

    Mode mode{Mode::log};

    std::vector<std::vector<float>> spectrogram;

    float x_spacing;
    float z_width;

    glm::vec3 position{0};

    std::unique_ptr<LoadContext> load_context;

    std::vector<HeightMapStrip> strips;
};

const float Waterfall::spacing{hop / samples_per_unit};

}  // namespace

//----------------------------------------------------------------------------//

class ImpulseRenderer::ContextLifetime : public BaseContextLifetime,
                                         public GLAudioThumbnailBase {
public:
    ContextLifetime()
            : waveform(generic_shader)
            , waterfall(fade_shader) {
    }

    void update(float dt) override {
        current_params.update(target_params);
        waveform.set_position(glm::vec3{0, 0, current_params.waveform_z});
        waterfall.set_position(
            glm::vec3{0, 0, current_params.waveform_z - 2.1});
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
        config_shader(fade_shader);

        auto s_shader = fade_shader.get_scoped();
        fade_shader.set_fade(current_params.fade);

        waveform.draw();
        waterfall.draw();
    }

    void set_mode(Mode u) {
        mode = u;

        switch (mode) {
            case Mode::waveform:
                target_params = waveform_params;
                break;
            case Mode::waterfall:
                target_params = waterfall_params;
                break;
        }
    }

    void mouse_down(const glm::vec2& pos) override {
        mousing = std::make_unique<Rotate>(target_params.azel, pos);
    }

    void mouse_drag(const glm::vec2& pos) override {
        assert(mousing);

        const auto& m = dynamic_cast<Rotate&>(*mousing);
        auto diff = pos - m.position;
        target_params.azel = Orientable::AzEl{
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

    //  inherited audio thumbnail stuff
    void clear() override {
        for (auto i : get_thumbnails()) {
            i->clear();
        }
    }
    void load_from(AudioFormatManager& manager, const File& file) override {
        for (auto i : get_thumbnails()) {
            i->load_from(manager, file);
        }
    }
    void reset(int num_channels,
               double sample_rate,
               int64 total_samples) override {
        for (auto i : get_thumbnails()) {
            i->reset(num_channels, sample_rate, total_samples);
        }
    }
    void addBlock(int64 sample_number_in_source,
                  const AudioSampleBuffer& new_data,
                  int start_offset,
                  int num_samples) override {
        for (auto i : get_thumbnails()) {
            i->addBlock(
                sample_number_in_source, new_data, start_offset, num_samples);
        }
    }

private:
    glm::mat4 get_projection_matrix() const {
        return glm::perspective(45.0f, get_aspect(), 0.05f, 1000.0f);
    }

    glm::mat4 get_rotation_matrix() const {
        auto i = glm::rotate(current_params.azel.azimuth, glm::vec3(0, 1, 0));
        auto j = glm::rotate(current_params.azel.elevation, glm::vec3(1, 0, 0));
        return j * i;
    }

    glm::mat4 get_view_matrix() const {
        return glm::lookAt(
                   glm::vec3{0, 0, eye}, glm::vec3{0}, glm::vec3{0, 1, 0}) *
               get_rotation_matrix();
    }

    std::array<GLAudioThumbnailBase*, 2> get_thumbnails() {
        return {&waveform, &waterfall};
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
    FadeShader fade_shader;

    struct ModeParams {
        Orientable::AzEl azel{0, 0};
        float fade{0};
        float waveform_z{0};

        void update(const ModeParams& target) {
            azel += (target.azel - azel) * 0.1;
            fade += (target.fade - fade) * 0.1;
            waveform_z += (target.waveform_z - waveform_z) * 0.1;
        }
    };

    static const ModeParams waveform_params;
    static const ModeParams waterfall_params;
    ModeParams current_params;
    ModeParams target_params;

    static const float angle_scale;

    float eye{4};

    Mode mode;
    Waveform waveform;
    Waterfall waterfall;

    std::unique_ptr<Mousing> mousing;
};

const ImpulseRenderer::ContextLifetime::ModeParams
    ImpulseRenderer::ContextLifetime::waveform_params{
        Orientable::AzEl{0, 0}, 0, 0};
const ImpulseRenderer::ContextLifetime::ModeParams
    ImpulseRenderer::ContextLifetime::waterfall_params{
        Orientable::AzEl{-0.7, 0.2}, 1, 2};

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

void ImpulseRenderer::clear() {
    std::lock_guard<std::mutex> lck(mut);
    push_incoming([this] { context_lifetime->clear(); });
}
void ImpulseRenderer::load_from(AudioFormatManager& manager, const File& file) {
    std::lock_guard<std::mutex> lck(mut);
    push_incoming(
        [this, &manager, file] { context_lifetime->load_from(manager, file); });
}
void ImpulseRenderer::reset(int num_channels,
                            double sample_rate,
                            int64 total_samples) {
    std::lock_guard<std::mutex> lck(mut);
    push_incoming([this, num_channels, sample_rate, total_samples] {
        context_lifetime->reset(num_channels, sample_rate, total_samples);
    });
}
void ImpulseRenderer::addBlock(int64 sample_number_in_source,
                               const AudioSampleBuffer& new_data,
                               int start_offset,
                               int num_samples) {
    std::lock_guard<std::mutex> lck(mut);
    push_incoming(
        [this, sample_number_in_source, new_data, start_offset, num_samples] {
            context_lifetime->addBlock(
                sample_number_in_source, new_data, start_offset, num_samples);
        });
}
