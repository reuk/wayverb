#include "ModelRenderer.hpp"

#include "boundaries.h"
#include "conversions.h"
#include "cl_common.h"
#include "tetrahedral_program.h"
#include "rayverb.h"

BoxObject::BoxObject(const GenericShader &shader)
        : BasicDrawableObject(shader,
                              {
                                  {-0.5, -0.5, -0.5},
                                  {-0.5, -0.5, 0.5},
                                  {-0.5, 0.5, -0.5},
                                  {-0.5, 0.5, 0.5},
                                  {0.5, -0.5, -0.5},
                                  {0.5, -0.5, 0.5},
                                  {0.5, 0.5, -0.5},
                                  {0.5, 0.5, 0.5},
                              },
                              std::vector<glm::vec4>(8, glm::vec4(1, 1, 0, 1)),
                              {
                                  0,
                                  1,
                                  1,
                                  3,
                                  3,
                                  2,
                                  2,
                                  0,

                                  4,
                                  5,
                                  5,
                                  7,
                                  7,
                                  6,
                                  6,
                                  4,

                                  0,
                                  4,
                                  1,
                                  5,
                                  2,
                                  6,
                                  3,
                                  7,
                              }) {
}

//----------------------------------------------------------------------------//

ModelSectionObject::ModelSectionObject(const GenericShader &shader,
                                       const SceneData &scene_data,
                                       const Octree &octree)
        : BasicDrawableObject(shader,
                              get_vertices(scene_data),
                              std::vector<glm::vec4>(scene_data.vertices.size(),
                                                     glm::vec4(1, 1, 1, 1)),
                              get_indices(scene_data, octree))
        , octree(octree) {
}

std::vector<glm::vec3> ModelSectionObject::get_vertices(
    const SceneData &scene_data) const {
    std::vector<glm::vec3> ret(scene_data.vertices.size());
    std::transform(scene_data.vertices.begin(),
                   scene_data.vertices.end(),
                   ret.begin(),
                   [](auto i) { return glm::vec3(i.x, i.y, i.z); });
    return ret;
}

void fill_indices_vector(const SceneData &scene_data,
                         const Octree &octree,
                         std::vector<GLuint> &ret) {
    for (const auto &i : octree.get_triangles()) {
        const auto &tri = scene_data.triangles[i];
        ret.push_back(tri.v0);
        ret.push_back(tri.v1);
        ret.push_back(tri.v2);
    }

    for (const auto &i : octree.get_nodes()) {
        fill_indices_vector(scene_data, i, ret);
    }
}

std::vector<GLuint> ModelSectionObject::get_indices(
    const SceneData &scene_data, const Octree &octree) const {
    std::vector<GLuint> ret;
    fill_indices_vector(scene_data, octree, ret);
    return ret;
}

void ModelSectionObject::draw_octree(const Octree &octree,
                                     BoxObject &box) const {
    if (octree.get_nodes().empty()) {
        auto aabb = octree.get_aabb();
        auto convert = [](auto i) { return glm::vec3(i.x, i.y, i.z); };

        box.set_scale(convert(aabb.get_dimensions()));
        box.set_position(convert(aabb.get_centre()));
        box.draw();
    }

    for (const auto &i : octree.get_nodes()) {
        draw_octree(i, box);
    }
}

void ModelSectionObject::draw() const {
    BasicDrawableObject::draw();
    BoxObject box(get_shader());
    draw_octree(octree, box);
}

//----------------------------------------------------------------------------//

ModelObject::ModelObject(const GenericShader &shader,
                         const SceneData &scene_data)
        : BasicDrawableObject(shader,
                              get_vertices(scene_data),
                              std::vector<glm::vec4>(scene_data.vertices.size(),
                                                     glm::vec4(1, 1, 1, 1)),
                              get_indices(scene_data)) {
}

std::vector<glm::vec3> ModelObject::get_vertices(
    const SceneData &scene_data) const {
    std::vector<glm::vec3> ret(scene_data.vertices.size());
    std::transform(scene_data.vertices.begin(),
                   scene_data.vertices.end(),
                   ret.begin(),
                   [](auto i) { return glm::vec3(i.x, i.y, i.z); });
    return ret;
}
std::vector<GLuint> ModelObject::get_indices(
    const SceneData &scene_data) const {
    std::vector<GLuint> ret(scene_data.triangles.size() * 3);
    auto count = 0u;
    for (const auto &tri : scene_data.triangles) {
        ret[count + 0] = tri.v0;
        ret[count + 1] = tri.v1;
        ret[count + 2] = tri.v2;
        count += 3;
    }
    return ret;
}

//----------------------------------------------------------------------------//

SphereObject::SphereObject(const GenericShader &shader,
                           const glm::vec3 &position,
                           const glm::vec4 &color)
        : BasicDrawableObject(shader,
                              {
                                  {0, 0, -1},
                                  {0, 0, 1},
                                  {0, -1, 0},
                                  {0, 1, 0},
                                  {-1, 0, 0},
                                  {1, 0, 0},
                              },
                              std::vector<glm::vec4>(6, color),
                              {
                                  0,
                                  2,
                                  4,
                                  0,
                                  2,
                                  5,
                                  0,
                                  3,
                                  4,
                                  0,
                                  3,
                                  5,
                                  1,
                                  2,
                                  4,
                                  1,
                                  2,
                                  5,
                                  1,
                                  3,
                                  4,
                                  1,
                                  3,
                                  5,
                              }) {
    set_position(position);
    set_scale(0.4);
}

//----------------------------------------------------------------------------//

MeshObject::MeshObject(const GenericShader &shader,
                       const TetrahedralWaveguide &waveguide)
        : shader(shader) {
    auto nodes = waveguide.get_mesh().get_nodes();
    std::vector<glm::vec3> v(nodes.size());
    std::transform(nodes.begin(),
                   nodes.end(),
                   v.begin(),
                   [](auto i) {
                       auto p = i.position;
                       return glm::vec3(p.x, p.y, p.z);
                   });

    //  init buffers
    std::vector<glm::vec4> c(v.size());
    std::transform(nodes.begin(),
                   nodes.end(),
                   c.begin(),
                   [](auto i) {
                       auto c = i.inside ? 1 : 0;
                       return glm::vec4(c, c, c, c);
                   });

    std::vector<GLuint> indices(v.size());
    std::iota(indices.begin(), indices.end(), 0);

    size = indices.size();

    geometry.data(v);
    colors.data(c);
    ibo.data(indices);

    //  init vao
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

void MeshObject::draw() const {
    auto s_shader = shader.get_scoped();
    shader.set_black(false);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_POINTS, size, GL_UNSIGNED_INT, nullptr);
}

void MeshObject::set_pressures(const std::vector<float> &pressures) {
    std::vector<glm::vec4> c(pressures.size(), glm::vec4(0, 0, 0, 0));
    std::transform(pressures.begin(),
                   pressures.end(),
                   c.begin(),
                   [this](auto i) {
                       auto p = i * amp;
                       switch (0) {
                           case 0:
                               return p > 0 ? glm::vec4(0, p, p, p)
                                            : glm::vec4(-p, 0, 0, -p);
                           case 1:
                               return glm::vec4(1, 1, 1, p);
                       }
                   });
    colors.data(c);
}

//----------------------------------------------------------------------------//

SceneRenderer::SceneRenderer()
        : projection_matrix(get_projection_matrix(1))
        , context(get_context())
        , device(get_device(context))
        , queue(context, device) {
}

SceneRenderer::~SceneRenderer() {
    if (future_pressure.valid())
        future_pressure.get();

    auto join_thread = [](auto &i) {
        if (i.joinable())
            i.join();
    };
    join_thread(waveguide_load_thread);
    join_thread(raytracer_load_thread);
}

void SceneRenderer::init_waveguide(const SceneData &scene_data,
                                   const WaveguideConfig &cc) {
    MeshBoundary boundary(scene_data);
    auto waveguide_program = get_program<TetrahedralProgram>(context, device);
    auto w = std::make_unique<TetrahedralWaveguide>(
        waveguide_program, queue, boundary, cc.get_divisions(), cc.get_mic());
    auto corrected_source = cc.get_source();

#define CASE 0
#if CASE == 0
    w->init(
        corrected_source, TetrahedralWaveguide::GaussianFunction(0.1), 0, 0);
#else
    corrected_source = w->get_corrected_coordinate(cc.get_source());
    w->init(corrected_source, TetrahedralWaveguide::BasicPowerFunction(), 0, 0);
#endif

    jassert((w->get_corrected_coordinate(cc.get_mic()) == cc.get_mic()).all());

    {
        std::lock_guard<std::mutex> lck(mut);
        waveguide = std::move(w);
        trigger_pressure_calculation();
    }
}

void SceneRenderer::init_raytracer(const SceneData &scene_data,
                                   const RayverbConfig &cc) {
    auto raytrace_program = get_program<RayverbProgram>(context, device);
    auto raytrace = std::make_unique<Raytrace>(
        raytrace_program, queue, cc.get_impulses(), scene_data);
    raytrace->raytrace(
        convert(cc.get_mic()), convert(cc.get_source()), cc.get_rays());
}

void SceneRenderer::newOpenGLContextCreated() {
    shader = std::make_unique<GenericShader>();

    File object(
        "/Users/reuben/dev/waveguide/demo/assets/test_models/vault.obj");
    File material(
        "/Users/reuben/dev/waveguide/demo/assets/materials/vault.json");
    File config("/Users/reuben/dev/waveguide/demo/assets/configs/vault.json");

    SceneData scene_data(object.getFullPathName().toStdString(),
                         material.getFullPathName().toStdString());
    CombinedConfig cc;
    try {
        cc = read_config(config.getFullPathName().toStdString());
    } catch (...) {
    }

    cc.get_source() = Vec3f(5, 1.75, 1);

    set_model_object(scene_data);
    set_config(cc);

    waveguide_load_thread =
        std::thread(&SceneRenderer::init_waveguide, this, scene_data, cc);
    raytracer_load_thread =
        std::thread(&SceneRenderer::init_raytracer, this, scene_data, cc);
}

void SceneRenderer::renderOpenGL() {
    std::lock_guard<std::mutex> lck(mut);

    if (waveguide && !mesh_object) {
        mesh_object = std::make_unique<MeshObject>(*shader, *waveguide);
    }

    if (future_pressure.valid() && waveguide && mesh_object) {
        try {
            if (future_pressure.wait_for(std::chrono::milliseconds(0)) ==
                std::future_status::ready) {
                mesh_object->set_pressures(future_pressure.get());
                trigger_pressure_calculation();
            }
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    }
    draw();
}

void SceneRenderer::trigger_pressure_calculation() {
    try {
        future_pressure = std::async(std::launch::async,
                                     [this] {
                                         auto ret = waveguide->run_step_slow();
                                         waveguide->swap_buffers();
                                         return ret;
                                     });
    } catch (...) {
        std::cout << "async error?" << std::endl;
    }
}

void SceneRenderer::openGLContextClosing() {
    model_object = nullptr;
    shader = nullptr;
}

glm::mat4 SceneRenderer::get_projection_matrix(float aspect) {
    return glm::perspective(45.0f, aspect, 0.05f, 1000.0f);
}

void SceneRenderer::set_aspect(float aspect) {
    std::lock_guard<std::mutex> lck(mut);
    projection_matrix = get_projection_matrix(aspect);
}

void SceneRenderer::draw() const {
    auto c = 0.25;
    glClearColor(c, c, c, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    //    glEnable(GL_PROGRAM_POINT_SIZE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto s_shader = shader->get_scoped();
    shader->set_model_matrix(glm::mat4());
    shader->set_view_matrix(get_view_matrix());
    shader->set_projection_matrix(get_projection_matrix());

    auto draw_thing = [this](const auto &i) {
        if (i)
            i->draw();
    };

    draw_thing(mesh_object);

    if (model_object) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        model_object->draw();

        draw_thing(source_object);
        draw_thing(receiver_object);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

glm::mat4 SceneRenderer::get_projection_matrix() const {
    return projection_matrix;
}
glm::mat4 SceneRenderer::get_view_matrix() const {
    auto rad = 20;
    glm::vec3 eye(0, 0, rad);
    glm::vec3 target(0, 0, 0);
    glm::vec3 up(0, 1, 0);
    auto mm = rotation * scale * translation;
    return glm::lookAt(eye, target, up) * mm;
}

void SceneRenderer::set_model_object(const SceneData &scene_data) {
    std::unique_lock<std::mutex> lck(mut);

    auto aabb = scene_data.get_aabb();
    auto m = aabb.get_centre();
    translation = glm::translate(-glm::vec3(m.x, m.y, m.z));

    auto max = (aabb.c1 - aabb.c0).max();
    auto s = max > 0 ? 20 / max : 1;
    scale = glm::scale(glm::vec3(s, s, s));

    Octree octree(scene_data, 0);
    model_object =
        std::make_unique<ModelSectionObject>(*shader, scene_data, octree);
}

void SceneRenderer::set_config(const Config &config) {
    std::unique_lock<std::mutex> lck(mut);

    auto s = config.get_source();
    glm::vec3 spos(s.x, s.y, s.z);
    auto r = config.get_mic();
    glm::vec3 rpos(r.x, r.y, r.z);

    source_object =
        std::make_unique<SphereObject>(*shader, spos, glm::vec4(1, 0, 0, 1));
    receiver_object =
        std::make_unique<SphereObject>(*shader, rpos, glm::vec4(0, 1, 1, 1));
}

void SceneRenderer::set_rotation(float az, float el) {
    std::lock_guard<std::mutex> lck(mut);
    auto i = glm::rotate(az, glm::vec3(0, 1, 0));
    auto j = glm::rotate(el, glm::vec3(1, 0, 0));
    rotation = j * i;
}