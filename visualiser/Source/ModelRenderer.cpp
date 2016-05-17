#include "ModelRenderer.hpp"

#include "MoreConversions.hpp"

#include "common/azimuth_elevation.h"
#include "common/boundaries.h"
#include "common/cl_common.h"
#include "common/conversions.h"
#include "common/json_read_write.h"
#include "common/kernel.h"

#include "combined_config_serialize.h"

RaytraceObject::RaytraceObject(const GenericShader &shader,
                               const RaytracerResults &results)
        : shader(shader) {
    auto impulses = results.impulses;
    std::vector<glm::vec3> v(impulses.size() + 1);
    v.front() = to_glm_vec3(results.source);
    std::transform(impulses.begin(), impulses.end(), v.begin() + 1, [](auto i) {
        return to_glm_vec3(i.position);
    });

    std::vector<glm::vec4> c(v.size());
    c.front() = glm::vec4(1, 1, 1, 1);
    std::transform(impulses.begin(), impulses.end(), c.begin() + 1, [](auto i) {
        auto average = std::accumulate(
                           std::begin(i.volume.s), std::end(i.volume.s), 0.0f) /
                       8;
        auto c = i.time ? fabs(average) * 10 : 0;
        return glm::vec4(c, c, c, c);
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

static constexpr auto model_colour = 0.75;

VoxelisedObject::VoxelisedObject(const GenericShader &shader,
                                 const SceneData &scene_data,
                                 const VoxelCollection &voxel)
        : BasicDrawableObject(
              shader,
              get_vertices(scene_data),
              std::vector<glm::vec4>(
                  scene_data.get_vertices().size(),
                  glm::vec4(
                      model_colour, model_colour, model_colour, model_colour)),
              get_indices(scene_data, voxel),
              GL_TRIANGLES)
        , voxel(voxel) {
}

std::vector<glm::vec3> VoxelisedObject::get_vertices(
    const SceneData &scene_data) const {
    std::vector<glm::vec3> ret(scene_data.get_vertices().size());
    std::transform(scene_data.get_vertices().begin(),
                   scene_data.get_vertices().end(),
                   ret.begin(),
                   [](auto i) { return to_glm_vec3(i); });
    return ret;
}

void fill_indices_vector(const SceneData &scene_data,
                         const VoxelCollection &voxel,
                         std::vector<GLuint> &ret) {
    for (const auto &x : voxel.get_data()) {
        for (const auto &y : x) {
            for (const auto &z : y) {
                for (const auto &i : z.get_triangles()) {
                    const auto &tri = scene_data.get_triangles()[i];
                    ret.push_back(tri.v0);
                    ret.push_back(tri.v1);
                    ret.push_back(tri.v2);
                }
            }
        }
    }
}

std::vector<GLuint> VoxelisedObject::get_indices(
    const SceneData &scene_data, const VoxelCollection &voxel) const {
    std::vector<GLuint> ret;
    fill_indices_vector(scene_data, voxel, ret);
    return ret;
}

void VoxelisedObject::draw() const {
    BasicDrawableObject::draw();
    BoxObject box(get_shader());

    for (const auto &x : voxel.get_data()) {
        for (const auto &y : x) {
            for (const auto &z : y) {
                if (!z.get_triangles().empty()) {
                    const auto &aabb = z.get_aabb();
                    box.set_scale(to_glm_vec3(aabb.dimensions()));
                    box.set_position(to_glm_vec3(aabb.centre()));
                    box.draw();
                }
            }
        }
    }
}

//----------------------------------------------------------------------------//

DrawableScene::DrawableScene(const GenericShader &shader,
                             const SceneData &scene_data,
                             const config::Combined &cc)
        : shader(shader)
        , context(get_context())
        , device(get_device(context))
        , queue(context, device)
        , model_object{std::make_unique<VoxelisedObject>(
              shader, scene_data, VoxelCollection(scene_data, 4, 0.1))}
        , source_object{std::make_unique<OctahedronObject>(
              shader, to_glm_vec3(cc.source), glm::vec4(1, 0, 0, 1))}
        , mic_object{std::make_unique<OctahedronObject>(
              shader, to_glm_vec3(cc.mic), glm::vec4(0, 1, 1, 1))}
        , future_waveguide{std::async(std::launch::async,
                                      &DrawableScene::init_waveguide,
                                      this,
                                      scene_data,
                                      cc)}
        , raytracer_results{std::async(std::launch::async,
                                       &DrawableScene::get_raytracer_results,
                                       this,
                                       scene_data,
                                       cc)} {
}

DrawableScene::~DrawableScene() noexcept {
    auto join_future = [](auto &i) {
        if (i.valid())
            i.get();
    };

    join_future(future_pressure);
    join_future(raytracer_results);
    join_future(future_waveguide);
}

std::unique_ptr<DrawableScene::Waveguide> DrawableScene::init_waveguide(
    const SceneData &scene_data, const config::Combined &cc) {
    MeshBoundary boundary(scene_data);
    auto waveguide_program =
        get_program<Waveguide::ProgramType>(context, device);

    auto w = std::make_unique<Waveguide>(waveguide_program,
                                         queue,
                                         boundary,
                                         cc.get_divisions(),
                                         cc.mic,
                                         cc.get_waveguide_sample_rate());
    auto corrected_source_index = w->get_index_for_coordinate(cc.source);
    auto corrected_source = w->get_coordinate_for_index(corrected_source_index);

    auto input = waveguide_kernel(cc.get_waveguide_sample_rate());

    w->init(corrected_source, std::move(input), 0);

    return w;
}

void DrawableScene::trigger_pressure_calculation() {
    try {
        future_pressure = std::async(
            std::launch::async, [this] { return waveguide->run_step_slow(); });
    } catch (...) {
        std::cout << "async error?" << std::endl;
    }
}

RaytracerResults DrawableScene::get_raytracer_results(
    const SceneData &scene_data, const config::Combined &cc) {
    auto raytrace_program = get_program<RaytracerProgram>(context, device);
    return Raytracer(raytrace_program, queue)
        .run(scene_data, cc.mic, cc.source, cc.rays, cc.impulses)
        .get_diffuse();
}

void DrawableScene::update(float dt) {
    std::lock_guard<std::mutex> lck(mut);

    if (!running) {
        return;
    }

    if (future_waveguide.valid()) {
        try {
            if (future_waveguide.wait_for(std::chrono::milliseconds(0)) ==
                std::future_status::ready) {
                waveguide = future_waveguide.get();
                trigger_pressure_calculation();
            }
        } catch (const std::exception &e) {
            std::cout << "exception fetching waveguide: " << e.what()
                      << std::endl;
        }
    }

    if (waveguide && !mesh_object) {
        mesh_object =
            std::make_unique<MeshObject<Waveguide>>(shader, *waveguide);
        /*
                std::cout << "showing mesh with " << waveguide->get_nodes()
                          << " nodes, of which: " << std::endl;
                std::cout << "  1d boundary nodes: "
                          << waveguide->get_mesh().compute_num_boundary<1>()
                          << std::endl;
                std::cout << "  2d boundary nodes: "
                          << waveguide->get_mesh().compute_num_boundary<2>()
                          << std::endl;
                std::cout << "  3d boundary nodes: "
                          << waveguide->get_mesh().compute_num_boundary<3>()
                          << std::endl;
        */
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
        draw_thing(mic_object);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void DrawableScene::set_mic(const Vec3f &u) {
    std::lock_guard<std::mutex> lck(mut);
    mic_object->set_position(to_glm_vec3(u));
}

void DrawableScene::set_source(const Vec3f &u) {
    std::lock_guard<std::mutex> lck(mut);
    source_object->set_position(to_glm_vec3(u));
}

void DrawableScene::start() {
    std::lock_guard<std::mutex> lck(mut);
    running = true;
}

void DrawableScene::stop() {
    std::lock_guard<std::mutex> lck(mut);
    running = false;
}

//----------------------------------------------------------------------------//

SceneRenderer::SceneRenderer(
    const SceneData &model,
    model::ValueWrapper<config::Combined> &config,
    model::ValueWrapper<model::RenderStateManager> &render_state_manager)
        : model(model)
        , config(config)
        , render_state_manager(render_state_manager)
        , projection_matrix(get_projection_matrix(1)) {
}

void SceneRenderer::newOpenGLContextCreated() {
    std::lock_guard<std::mutex> lck(mut);
    shader = std::make_unique<GenericShader>();
    scene = std::make_unique<DrawableScene>(*shader, model, config);

    auto aabb = model.get_aabb();
    auto m = aabb.centre();
    auto max = aabb.dimensions().max();
    scale = max > 0 ? 20 / max : 1;
    translation = glm::translate(-glm::vec3(m.x, m.y, m.z));
}

void SceneRenderer::renderOpenGL() {
    std::lock_guard<std::mutex> lck(mut);

    update();
    draw();
}

void SceneRenderer::openGLContextClosing() {
    std::lock_guard<std::mutex> lck(mut);
    scene = nullptr;
    shader = nullptr;
}

glm::mat4 SceneRenderer::get_projection_matrix(float aspect) {
    return glm::perspective(45.0f, aspect, 0.05f, 1000.0f);
}

void SceneRenderer::set_aspect(float aspect) {
    std::lock_guard<std::mutex> lck(mut);
    projection_matrix = get_projection_matrix(aspect);
}

void SceneRenderer::update() {
    if (render_state_manager.state == model::RenderState::started) {
        render_state_manager.progress.set(render_state_manager.progress +
                                          0.001);
    }
    if (scene) {
        scene->update(0);
    }
}

void SceneRenderer::draw() const {
    auto c = 0.0;
    glClearColor(c, c, c, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

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

void SceneRenderer::start() {
    std::lock_guard<std::mutex> lck(mut);
    if (scene) {
        scene->start();
    }
}

void SceneRenderer::stop() {
    std::lock_guard<std::mutex> lck(mut);
    if (scene) {
        scene->stop();
    }
}

void SceneRenderer::changeListenerCallback(ChangeBroadcaster *cb) {
    std::lock_guard<std::mutex> lck(mut);
    if (cb == &config.mic) {
        if (scene) {
            scene->set_mic(config.mic);
        }
    } else if (cb == &config.source) {
        if (scene) {
            scene->set_source(config.source);
        }
    }
}