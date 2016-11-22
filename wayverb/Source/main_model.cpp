#include "main_model.h"

#include "UtilityComponents/async_work_queue.h"

#include "core/cl/common.h"
#include "core/serialize/range.h"
#include "core/serialize/surface.h"

#include "cereal/archives/json.hpp"
#include "cereal/types/memory.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/tuple.hpp"

#include <fstream>

project::project(const std::string& fpath)
        : scene_data_{is_project_file(fpath) ? compute_model_path(fpath)
                                             : fpath}
        , needs_save_{!is_project_file(fpath)} {
    //  First make sure default source and receiver are in a sensible position.
    const auto aabb =
            wayverb::core::geo::compute_aabb(get_scene_data().get_vertices());
    const auto c = centre(aabb);

    (*persistent.sources())[0]->set_position(c);
    (*persistent.receivers())[0]->set_position(c);

    //  Set up surfaces.
    const auto& surface_strings = scene_data_.get_scene_data()->get_surfaces();
    *persistent.materials() = wayverb::combined::model::materials_from_names<1>(begin(surface_strings),
                                                      end(surface_strings));

    //  If there's a config file, we'll just overwrite the state we just set,
    //  but that's probably fine.
    if (is_project_file(fpath)) {
        const auto config_file = project::compute_config_path(fpath);

        //  load the config
        std::ifstream stream{config_file};
        cereal::JSONInputArchive{stream}(persistent);
    }

    persistent.connect([&](auto&) { needs_save_ = true; });
}

std::string project::compute_model_path(const std::string& root) {
    return root + '/' + model_name;
}

std::string project::compute_config_path(const std::string& root) {
    return root + '/' + config_name;
}

bool project::is_project_file(const std::string& fpath) {
    return std::string{std::find_if(crbegin(fpath),
                                    crend(fpath),
                                    [](auto i) { return i == '.'; })
                               .base(),
                       end(fpath)} == project_extension;
}

std::string project::get_extensions() const {
    return scene_data_.get_extensions() + ';' + project_extension;
}

void project::save_to(const std::string& fpath) {
    if (needs_save_) {
        //  TODO create directory

        //  write current geometry to file
        scene_data_.save(project::compute_model_path(fpath));

        //  write config with all current materials to file
        std::ofstream stream{project::compute_config_path(fpath)};
        cereal::JSONOutputArchive{stream}(persistent);

        needs_save_ = false;
    }
}

bool project::needs_save() const { return needs_save_; }

wayverb::core::generic_scene_data<cl_float3, std::string> project::get_scene_data()
        const {
    return *scene_data_.get_scene_data();
}

////////////////////////////////////////////////////////////////////////////////

namespace detail {
template <typename T>
class queue_forwarding_call final {
public:
    constexpr queue_forwarding_call(T& t, async_work_queue& queue)
            : t_{t}
            , queue_{queue} {}

    template <typename... Ts>
    void operator()(Ts... ts) const {
        this->queue_.push([ t = &t_, ts... ] { (*t)(ts...); });
    }

private:
    T& t_;
    async_work_queue& queue_;
};

template <typename T>
auto make_queue_forwarding_call(T& t, async_work_queue& queue) {
    return queue_forwarding_call<T>{t, queue};
}

}  // namespace detail

class main_model::impl final {
    template <typename T>
    auto make_queue_forwarding_call(T& t) {
        return detail::make_queue_forwarding_call(t, queue_);
    }

public:
    impl()
        : begun_connection_{engine_.connect_begun(
                  make_queue_forwarding_call(begun_))}
        , engine_state_changed_connection_{engine_.connect_engine_state_changed(
                  make_queue_forwarding_call(engine_state_changed_))}
        , node_positions_changed_connection_{engine_.connect_waveguide_node_positions_changed(
                  make_queue_forwarding_call(node_positions_changed_))}
        , node_pressures_changed_connection_{engine_.connect_waveguide_node_pressures_changed(
                  make_queue_forwarding_call(node_pressures_changed_))}
        , reflections_generated_connection_{engine_.connect_raytracer_reflections_generated(
                  make_queue_forwarding_call(reflections_generated_))}
        , encountered_error_connection_{engine_.connect_encountered_error(
                  make_queue_forwarding_call(encountered_error_))}
        , finished_connection_{engine_.connect_finished(
                  make_queue_forwarding_call(finished_))} {}

    ~impl() noexcept { cancel_render(); }

    void start_render(const class project& project, const wayverb::combined::model::output& output) {
        engine_.run(wayverb::core::compute_context{},
                    generate_scene_data(project),
                    project.persistent,
                    output);
    }

    void cancel_render() { engine_.cancel(); }

    bool is_rendering() const { return engine_.is_running(); }
    
    engine_state_changed::connection connect_engine_state(
            engine_state_changed::callback_type t) {
        return engine_state_changed_.connect(std::move(t));
    }

    waveguide_node_positions_changed::connection connect_node_positions(
            waveguide_node_positions_changed::callback_type t) {
        return node_positions_changed_.connect(std::move(t));
    }

    waveguide_node_pressures_changed::connection connect_node_pressures(
            waveguide_node_pressures_changed::callback_type t) {
        return node_pressures_changed_.connect(std::move(t));
    }

    raytracer_reflections_generated::connection connect_reflections(
            raytracer_reflections_generated::callback_type t) {
        return reflections_generated_.connect(std::move(t));
    }

    encountered_error::connection connect_error_handler(
            encountered_error::callback_type t) {
        return encountered_error_.connect(std::move(t));
    }

    begun::connection connect_begun(begun::callback_type t) {
        return begun_.connect(std::move(t));
    }

    finished::connection connect_finished(finished::callback_type t) {
        return finished_.connect(std::move(t));
    }

private:
    wayverb::core::gpu_scene_data generate_scene_data(const class project& project) const {
        util::aligned::unordered_map<std::string,
                                     wayverb::core::surface<wayverb::core::simulation_bands>>
                material_map;

        for (const auto& i : *project.persistent.materials()) {
            material_map[i->get_name()] = i->get_surface();
        }

        return scene_with_extracted_surfaces(project.get_scene_data(),
                                             material_map);
    }

    async_work_queue queue_;

    wayverb::combined::complete_engine engine_;

    begun begun_;
    begun::scoped_connection begun_connection_;
    
    engine_state_changed engine_state_changed_;
    engine_state_changed::scoped_connection engine_state_changed_connection_;
    
    waveguide_node_positions_changed node_positions_changed_;
    waveguide_node_positions_changed::scoped_connection
            node_positions_changed_connection_;

    waveguide_node_pressures_changed node_pressures_changed_;
    waveguide_node_pressures_changed::scoped_connection
            node_pressures_changed_connection_;

    raytracer_reflections_generated reflections_generated_;
    raytracer_reflections_generated::scoped_connection
            reflections_generated_connection_;

    encountered_error encountered_error_;
    encountered_error::scoped_connection encountered_error_connection_;

    finished finished_;
    finished::scoped_connection finished_connection_;
};

////////////////////////////////////////////////////////////////////////////////

main_model::main_model(const std::string& name)
        : project{name}
        , material_presets{wayverb::combined::model::presets::materials}
        , capsule_presets{wayverb::combined::model::presets::capsules}
        , currently_open_file_{name}
        , pimpl_{std::make_unique<impl>()} {}

main_model::~main_model() noexcept = default;

void main_model::start_render() {
    pimpl_->start_render(project, output);
}

void main_model::cancel_render() { pimpl_->cancel_render(); }

bool main_model::is_rendering() const { return pimpl_->is_rendering(); }

void main_model::save(const save_callback& callback) {
    if (project::is_project_file(currently_open_file_)) {
        project.save_to(currently_open_file_);
    } else {
        if (const auto fpath = callback()) {
            save_as(*fpath);
        }
    }
}

void main_model::save_as(std::string name) {
    if (!project::is_project_file(name)) {
        throw std::runtime_error{
                "Save path must have correct project extension."};
    }
    project.save_to(name);
    currently_open_file_ = std::move(name);
}

bool main_model::needs_save() const { return project.needs_save(); }

//  CALLBACKS  /////////////////////////////////////////////////////////////////

main_model::begun::connection main_model::connect_begun(
        begun::callback_type t) {
    return pimpl_->connect_begun(std::move(t));
}

main_model::engine_state_changed::connection main_model::connect_engine_state(
        engine_state_changed::callback_type t) {
    return pimpl_->connect_engine_state(std::move(t));
}

main_model::waveguide_node_positions_changed::connection
main_model::connect_node_positions(
        waveguide_node_positions_changed::callback_type t) {
    return pimpl_->connect_node_positions(std::move(t));
}

main_model::waveguide_node_pressures_changed::connection
main_model::connect_node_pressures(
        waveguide_node_pressures_changed::callback_type t) {
    return pimpl_->connect_node_pressures(std::move(t));
}

main_model::raytracer_reflections_generated::connection
main_model::connect_reflections(
        raytracer_reflections_generated::callback_type t) {
    return pimpl_->connect_reflections(std::move(t));
}

main_model::encountered_error::connection main_model::connect_error_handler(
        encountered_error::callback_type t) {
    return pimpl_->connect_error_handler(std::move(t));
}

main_model::finished::connection main_model::connect_finished(
        finished::callback_type t) {
    return pimpl_->connect_finished(std::move(t));
}

//  MISC FUNCTIONS  ////////////////////////////////////////////////////////////

void main_model::reset_view() {
    //  Set up the scene model so that everything is visible.
    const auto scene_data = project.get_scene_data();
    const auto vertices = util::map_to_vector(begin(scene_data.get_vertices()),
                                              end(scene_data.get_vertices()),
                                              wayverb::core::to_vec3{});

    const auto aabb = wayverb::core::geo::compute_aabb(vertices);
    const auto origin = -centre(aabb);
    const auto radius = glm::distance(aabb.get_min(), aabb.get_max()) / 2;

    scene.set_origin(origin);
    scene.set_eye_distance(2 * radius);
    scene.set_rotation(wayverb::core::az_el{M_PI / 4, M_PI / 6});
}
