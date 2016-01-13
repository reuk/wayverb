#include "ModelRenderer.hpp"

#include "boundaries.h"
#include "conversions.h"
#include "cl_common.h"
#include "tetrahedral_program.h"

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

auto model_colour = 0.5;
ModelSectionObject::ModelSectionObject(const GenericShader &shader,
                                       const SceneData &scene_data,
                                       const Octree &octree)
        : BasicDrawableObject(
              shader,
              get_vertices(scene_data),
              std::vector<glm::vec4>(
                  scene_data.vertices.size(),
                  glm::vec4(
                      model_colour, model_colour, model_colour, model_colour)),
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
    if (!octree.get_triangles().empty() && octree.get_nodes().empty()) {
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
        : BasicDrawableObject(
              shader,
              get_vertices(scene_data),
              std::vector<glm::vec4>(
                  scene_data.vertices.size(),
                  glm::vec4(
                      model_colour, model_colour, model_colour, model_colour)),
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

template <typename T>
auto to_glm_vec3(const T &t) {
    return glm::vec3(t.x, t.y, t.z);
}

RaytraceObject::RaytraceObject(const GenericShader &shader,
                               const RaytracerResults &results)
        : shader(shader) {
    auto impulses = results.impulses;
    std::vector<glm::vec3> v(impulses.size() + 1);
    v.front() = to_glm_vec3(results.source);
    std::transform(impulses.begin(),
                   impulses.end(),
                   v.begin() + 1,
                   [](auto i) { return to_glm_vec3(i.position); });

    std::vector<glm::vec4> c(v.size());
    c.front() = glm::vec4(1, 1, 1, 1);
    std::transform(impulses.begin(),
                   impulses.end(),
                   c.begin() + 1,
                   [](auto i) {
                       auto average = std::accumulate(std::begin(i.volume.s),
                                                      std::end(i.volume.s),
                                                      0.0f) /
                                      8;
                       auto c = i.time ? fabs(average) * 10 : 0;
                       //                       auto c = 1;
                       return glm::vec4(0, c, c, c);
                   });

    std::vector<std::pair<GLuint, GLuint>> lines;
    for (auto ray = 0u; ray != results.rays; ++ray) {
        auto prev = 0u;
        for (auto pt = 0u; pt != results.rays; ++pt) {
            auto index = ray * results.reflections + pt + 1;
            lines.push_back(std::make_pair(prev, index));
            prev = index;
        }
    }

    std::vector<GLuint> indices;
    for (const auto &i : lines) {
        indices.push_back(i.first);
        indices.push_back(i.second);
    }

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

void RaytraceObject::draw() const {
    auto s_shader = shader.get_scoped();
    shader.set_black(false);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_LINES, size, GL_UNSIGNED_INT, nullptr);
}

//----------------------------------------------------------------------------//

DrawableScene::DrawableScene(const GenericShader &shader,
                             const SceneData &scene_data,
                             const CombinedConfig &cc)
        : shader(shader)
        , context(get_context())
        , device(get_device(context))
        , queue(context, device)
        , model_object{std::make_unique<ModelSectionObject>(
              shader, scene_data, Octree(scene_data, 0, 0.1))}
        , source_object{std::make_unique<SphereObject>(
              shader, to_glm_vec3(cc.get_source()), glm::vec4(1, 0, 0, 1))}
        , receiver_object{std::make_unique<SphereObject>(
              shader, to_glm_vec3(cc.get_mic()), glm::vec4(0, 1, 1, 1))}
        , waveguide_load_thread{&DrawableScene::init_waveguide,
                                this,
                                scene_data,
                                cc}
        , raytracer_results{
              std::move(std::async(std::launch::async,
                                   &DrawableScene::get_raytracer_results,
                                   this,
                                   scene_data,
                                   cc))} {
}

DrawableScene::~DrawableScene() noexcept {
    auto join_future = [](auto &i) {
        if (i.valid())
            i.get();
    };
    auto join_thread = [](auto &i) {
        if (i.joinable())
            i.join();
    };

    join_future(future_pressure);
    join_future(raytracer_results);
    join_thread(waveguide_load_thread);
}

void DrawableScene::init_waveguide(const SceneData &scene_data,
                                   const WaveguideConfig &cc) {
    MeshBoundary boundary(scene_data);
    auto waveguide_program = get_program<TetrahedralProgram>(context, device);
    auto w = std::make_unique<TetrahedralWaveguide>(
        waveguide_program, queue, boundary, cc.get_divisions(), cc.get_mic());
    auto corrected_source = cc.get_source();

    w->init(
        corrected_source, TetrahedralWaveguide::GaussianFunction(0.1), 0, 0);

    {
        std::lock_guard<std::mutex> lck(mut);
        waveguide = std::move(w);
        trigger_pressure_calculation();
    }
}

void DrawableScene::trigger_pressure_calculation() {
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

RaytracerResults DrawableScene::get_raytracer_results(
    const SceneData &scene_data, const CombinedConfig &cc) {
    auto raytrace_program = get_program<RayverbProgram>(context, device);
    return ImprovedRaytrace(raytrace_program, queue)
        .BaseRaytrace::run(scene_data,
                           cc.get_mic(),
                           cc.get_source(),
                           cc.get_rays(),
                           cc.get_impulses())
        .get_diffuse();
}

void DrawableScene::update(float dt) {
    std::lock_guard<std::mutex> lck(mut);

    if (waveguide && !mesh_object) {
        mesh_object = std::make_unique<MeshObject>(shader, *waveguide);
        std::cout << "showing mesh with " << waveguide->get_nodes() << " nodes"
                  << std::endl;
    }

    if (raytracer_results.valid()) {
        try {
            if (raytracer_results.wait_for(std::chrono::milliseconds(0)) ==
                std::future_status::ready) {
                raytrace_object = std::make_unique<RaytraceObject>(
                    shader, raytracer_results.get());
            }
        } catch (const std::exception &e) {
            std::cout << "exception fetching raytracer results: " << e.what()
                      << std::endl;
        }
    }

    if (future_pressure.valid() && waveguide && mesh_object) {
        try {
            if (future_pressure.wait_for(std::chrono::milliseconds(0)) ==
                std::future_status::ready) {
                mesh_object->set_pressures(future_pressure.get());
                trigger_pressure_calculation();
            }
        } catch (const std::exception &e) {
            std::cout << "exception fetching waveguide results: " << e.what()
                      << std::endl;
        }
    }
}

void DrawableScene::draw() const {
    std::lock_guard<std::mutex> lck(mut);

    auto s_shader = shader.get_scoped();
    auto draw_thing = [this](const auto &i) {
        if (i)
            i->draw();
    };

    draw_thing(mesh_object);
    draw_thing(raytrace_object);

    if (model_object) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        model_object->draw();

        draw_thing(source_object);
        draw_thing(receiver_object);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

//----------------------------------------------------------------------------//

SceneRenderer::SceneRenderer()
        : projection_matrix(get_projection_matrix(1)) {
}

SceneRenderer::~SceneRenderer() {
}

void SceneRenderer::load_from_file_package(const FilePackage &fp) {
    std::lock_guard<std::mutex> lck(mut);

    SceneData scene_data(fp.get_object().getFullPathName().toStdString(),
                         fp.get_material().getFullPathName().toStdString());
    CombinedConfig cc;
    try {
        cc = read_config(fp.get_config().getFullPathName().toStdString());
    } catch (...) {
    }

    cc.get_rays() = 1 << 5;

    scene = std::make_unique<DrawableScene>(*shader, scene_data, cc);

    auto aabb = scene_data.get_aabb();
    auto m = aabb.get_centre();
    auto max = (aabb.c1 - aabb.c0).max();
    auto s = max > 0 ? 20 / max : 1;

    translation = glm::translate(-glm::vec3(m.x, m.y, m.z));
    scale = s;
}

void SceneRenderer::newOpenGLContextCreated() {
    shader = std::make_unique<GenericShader>();
}

void SceneRenderer::renderOpenGL() {
    std::lock_guard<std::mutex> lck(mut);

    update();
    draw();
}

void SceneRenderer::openGLContextClosing() {
}

glm::mat4 SceneRenderer::get_projection_matrix(float aspect) {
    return glm::perspective(45.0f, aspect, 0.05f, 1000.0f);
}

void SceneRenderer::set_aspect(float aspect) {
    std::lock_guard<std::mutex> lck(mut);
    projection_matrix = get_projection_matrix(aspect);
}

void SceneRenderer::update() {
    if (scene)
        scene->update(0);
}

void SceneRenderer::draw() const {
    auto c = 0.0;
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

    draw_thing(scene);
}

glm::mat4 SceneRenderer::get_projection_matrix() const {
    return projection_matrix;
}
glm::mat4 SceneRenderer::get_view_matrix() const {
    auto rad = 20;
    glm::vec3 eye(0, 0, rad);
    glm::vec3 target(0, 0, 0);
    glm::vec3 up(0, 1, 0);
    auto mm = rotation * get_scale_matrix() * translation;
    return glm::lookAt(eye, target, up) * mm;
}

void SceneRenderer::set_rotation(float az, float el) {
    std::lock_guard<std::mutex> lck(mut);
    auto i = glm::rotate(az, glm::vec3(0, 1, 0));
    auto j = glm::rotate(el, glm::vec3(1, 0, 0));
    rotation = j * i;
}

void SceneRenderer::update_scale(float delta) {
    std::lock_guard<std::mutex> lck(mut);
    scale = std::max(0.0f, scale + delta);
}

glm::mat4 SceneRenderer::get_scale_matrix() const {
    return glm::scale(glm::vec3(scale, scale, scale));
}