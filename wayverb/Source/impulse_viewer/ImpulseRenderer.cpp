#include "AxesObject.hpp"
#include "BasicDrawableObject.hpp"
#include "ImpulseRenderer.hpp"

#include "common/stl_wrappers.h"

#include "modern_gl_utils/generic_shader.h"

namespace {

//  might need an async solution if this is really slow...
auto load_test_file() {
    AudioFormatManager m;
    m.registerBasicFormats();
    std::unique_ptr<AudioFormatReader> reader(
        m.createReaderFor(File("/Users/reuben/dev/pyverb/impulse.aiff")));

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

class Waveform : public BasicDrawableObject {
public:
    Waveform(const GenericShader& shader,
             const std::vector<std::pair<float, float>>& data)
            : Waveform(shader, compute_geometry(data)) {
    }

private:
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
        auto step = 0.01;
        auto x = 0.0;
        for (const auto& i : data) {
            ret.push_back(glm::vec3{x, i.first, 0});
            ret.push_back(glm::vec3{x, i.second, 0});
            x += step;
        }
        return ret;
    }
};

class HeightMap : public ::Drawable {
public:
    HeightMap(const std::vector<std::vector<float>>& heights);

private:
    class HeightMapStrip : public ::Drawable {
    public:
        HeightMapStrip(const GenericShader& shader,
                       const std::vector<float>& left,
                       const std::vector<float>& right,
                       float x,
                       float x_spacing,
                       float z_spacing)
                : shader(shader) {
            auto g = compute_geometry(left, right, x, x_spacing, z_spacing);
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
            float x,
            float x_spacing,
            float z_spacing) {
            assert(left.size() == right.size());

            auto count = left.size();

            std::vector<glm::vec3> ret(count * 2);
            for (auto i = 0u, j = 0u; i != count; ++i, j += 2) {
                ret[j + 0] = glm::vec3{x, left[i], z_spacing * i};
                ret[j + 1] = glm::vec3{x + x_spacing, right[i], z_spacing * i};
            }
            return ret;
        }

        static std::vector<glm::vec4> compute_colors(
            const std::vector<glm::vec3>& g) {
            std::vector<glm::vec4> ret(g.size());
            proc::transform(g, ret.begin(), [](const auto& i) {
                return glm::mix(
                    glm::vec4{0.4, 0.4, 0.4, 1}, glm::vec4{1, 1, 1, 1}, i.y);
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
};

class Waterfall : public BasicDrawableObject {
public:
private:
};

}  // namespace

class ImpulseRenderer::ContextLifetime : public BaseContextLifetime {
public:
    ContextLifetime()
            : axes(generic_shader)
            , waveform(generic_shader,
                       compute_downsampled_waveform(load_test_file()).front()) {
        waveform.set_position(glm::vec3{0, 0, 1});
    }

    void update(float dt) override {
        current_azel += (target_azel - current_azel) * 0.1;
    }

    void draw() const override {
        auto c = 0.0;
        glClearColor(c, c, c, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        /*
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POLYGON_SMOOTH);
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

        axes.draw();
        waveform.draw();
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

    static const int waveform_steps = 256;

    static std::vector<std::vector<std::pair<float, float>>>
    compute_downsampled_waveform(const std::vector<std::vector<float>>& in) {
        std::vector<std::vector<std::pair<float, float>>> ret(in.size());
        proc::transform(in, ret.begin(), [](const auto& i) {
            std::vector<std::pair<float, float>> ret(i.size() / waveform_steps);
            for (auto a = 0u, b = 0u; a < i.size() - waveform_steps;
                 a += waveform_steps, ++b) {
                auto mm = std::minmax_element(i.begin() + a,
                                              i.begin() + a + waveform_steps);
                ret[b] = std::make_pair(*mm.first, *mm.second);
            }
            return ret;
        });
        return ret;
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
    GenericShader generic_shader;
    AxesObject axes;

    static const Orientable::AzEl waveform_azel;
    static const Orientable::AzEl waterfall_azel;

    Orientable::AzEl current_azel{0, 0};
    Orientable::AzEl target_azel{0, 0};
    float eye{4};

    Mode mode;
    Waveform waveform;
};

const Orientable::AzEl ImpulseRenderer::ContextLifetime::waveform_azel{0, 0};
const Orientable::AzEl ImpulseRenderer::ContextLifetime::waterfall_azel{-0.5,
                                                                        0.4};

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