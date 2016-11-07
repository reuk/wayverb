#include "combined/app_model.h"

namespace wayverb {
namespace combined {

source_model::source_model(core::geo::box bounds)
        : bounds_{std::move(bounds)}
        , data_{"new source", centre(bounds_)} {}

void source_model::set_name(std::string name) {
    data_.name = std::move(name);
    on_change_(*this);
}

std::string source_model::get_name() const { return data_.name; }

void source_model::set_position(const glm::vec3& position) {
    data_.position = clamp(position, bounds_);
    on_change_(*this);
}

glm::vec3 source_model::get_position() const { return data_.position; }

source_info source_model::get_raw() const { return data_; }

source_model::on_change::connection source_model::connect_on_change(
        on_change::callback_type t) {
    return on_change_.connect(std::move(t));
}

////////////////////////////////////////////////////////////////////////////////

void microphone_model::set_orientation(float azimuth, float elevation) {
    microphone_.set_pointing(compute_pointing(core::az_el{azimuth, elevation}));
    on_change_(*this);
}

void microphone_model::set_shape(double shape) {
    microphone_.set_shape(shape);
    on_change_(*this);
}

core::attenuator::microphone microphone_model::get_raw() const {
    return microphone_;
}

microphone_model::on_change::connection microphone_model::connect_on_change(
        on_change::callback_type t) {
    return on_change_.connect(std::move(t));
}

void hrtf_model::set_orientation(float azimuth, float elevation) {
    hrtf_.set_pointing(compute_pointing(core::az_el{azimuth, elevation}));
    on_change_(*this);
}

void hrtf_model::set_channel(core::attenuator::hrtf::channel channel) {
    hrtf_.set_channel(channel);
    on_change_(*this);
}

core::attenuator::hrtf hrtf_model::get_raw() const { return hrtf_; }

hrtf_model::on_change::connection hrtf_model::connect_on_change(
        on_change::callback_type t) {
    return on_change_.connect(std::move(t));
}

////////////////////////////////////////////////////////////////////////////////

capsule_model::capsule_model() {
    microphone.connect_on_change([&](auto&) { on_change_(*this); });
    hrtf.connect_on_change([&](auto&) { on_change_(*this); });
}

void capsule_model::set_type(type type) {
    type_ = type;
    on_change_(*this);
}

void capsule_model::set_name(std::string name) {
    name_ = name;
    on_change_(*this);
}

capsule_model::on_change::connection capsule_model::connect_on_change(
        on_change::callback_type t) {
    return on_change_.connect(std::move(t));
}

////////////////////////////////////////////////////////////////////////////////

receiver_model::receiver_model(core::geo::box bounds)
        : bounds_{std::move(bounds)}
        , name_{"new receiver"}
        , position_{centre(bounds_)} {
    capsules_.add(0);
    capsules_.connect_on_change([&](auto&) { on_change_(*this); });
}

void receiver_model::set_name(std::string name) {
    name_ = std::move(name);
    on_change_(*this);
}

void receiver_model::set_position(const glm::vec3& position) {
    position_ = clamp(position, bounds_);
    on_change_(*this);
}

void receiver_model::set_orientation(float azimuth, float elevation) {
    orientation_.set_pointing(
            compute_pointing(core::az_el{azimuth, elevation}));
    on_change_(*this);
}

receiver_model::on_change::connection receiver_model::connect_on_change(
        on_change::callback_type t) {
    return on_change_.connect(std::move(t));
}

const capsule_model& receiver_model::get_capsule(size_t index) const {
    return capsules_[index];
}

capsule_model& receiver_model::get_capsule(size_t index) {
    return capsules_[index];
}

void receiver_model::add_capsule(size_t index) {
    capsules_.add(index);
}

void receiver_model::remove_capsule(size_t index) {
    if (1 < capsules_.size()) {
        capsules_.remove(index);
    }
}

////////////////////////////////////////////////////////////////////////////////

void raytracer_model::set_rays(size_t rays) {
    rays_ = rays;
    on_change_(*this);
}

void raytracer_model::set_max_img_src_order(size_t max) {
    max_img_src_order_ = max;
    on_change_(*this);
}

raytracer_model::on_change::connection raytracer_model::connect_on_change(
        on_change::callback_type t) {
    return on_change_.connect(std::move(t));
}

void single_band_waveguide_model::set_cutoff(double cutoff) {
    cutoff_ = cutoff;
    on_change_(*this);
}

void single_band_waveguide_model::set_usable_portion(double usable) {
    usable_portion_ = clamp(usable, util::make_range(0.0, 1.0));
    on_change_(*this);
}

single_band_waveguide_model::on_change::connection
single_band_waveguide_model::connect_on_change(on_change::callback_type t) {
    return on_change_.connect(std::move(t));
}

void multiple_band_waveguide_model::set_bands(size_t bands) {
    bands_ = clamp(bands, util::make_range(size_t{1}, size_t{8}));
    maintain_valid_cutoff();
    on_change_(*this);
}

void multiple_band_waveguide_model::set_cutoff(double cutoff) {
    cutoff_ = cutoff;
    maintain_valid_cutoff();
    on_change_(*this);
}

void multiple_band_waveguide_model::set_usable_portion(double usable) {
    usable_portion_ = usable;
    on_change_(*this);
}

multiple_band_waveguide_model::on_change::connection
multiple_band_waveguide_model::connect_on_change(on_change::callback_type t) {
    return on_change_.connect(std::move(t));
}

void multiple_band_waveguide_model::maintain_valid_cutoff() {
    cutoff_ = std::max(cutoff_, band_params_.edges[bands_]);
}

waveguide_model::waveguide_model() {
    single_band.connect_on_change([&](auto&) { on_change_(*this); });
    multiple_band.connect_on_change([&](auto&) { on_change_(*this); });
}

void waveguide_model::set_type(type type) {
    type_ = type;
    on_change_(*this);
}

waveguide_model::on_change::connection waveguide_model::connect_on_change(
        on_change::callback_type t) {
    return on_change_.connect(std::move(t));
}

////////////////////////////////////////////////////////////////////////////////

void output_model::set_output_folder(std::string output_folder) {
    output_folder_ = std::move(output_folder);
    on_change_(*this);
}

void output_model::set_name(std::string name) {
    name_ = std::move(name);
    on_change_(*this);
}

void output_model::set_sample_rate(double sr) {
    sample_rate_ = sr;
    on_change_(*this);
}

void output_model::set_bit_depth(bit_depth bit_depth) {
    bit_depth_ = bit_depth;
    on_change_(*this);
}

output_model::on_change::connection output_model::connect_on_change(
        on_change::callback_type t) {
    return on_change_.connect(std::move(t));
}

////////////////////////////////////////////////////////////////////////////////

app_model::app_model(const std::string& name)
        : aabb_{core::geo::compute_aabb(scene_.get_scene_data())} {
    const auto callback = [&](auto&) { on_change_(*this); };

    add_source(0);
    add_receiver(0);

    raytracer.connect_on_change(callback);
    waveguide.connect_on_change(callback);
    output.connect_on_change(callback);
    sources_.connect_on_change(callback);
    receivers_.connect_on_change(callback);
}

app_model::~app_model() noexcept { stop_render(); }

//  RENDERING
//  /////////////////////////////////////////////////////////////
void app_model::start_render() {
    stop_render();

    //  Collect parameters.

    auto scene_data = scene_.get_scene_data();
    //  TODO replace surfaces with the surfaces from the model.

    //  Start engine in new thread.
    future_ = std::async(std::launch::async, [&] {
        engine_.run(core::compute_context{},
                    scene_data,
                    make_scene_parameters(

                            ));
    });
}

void app_model::stop_render() {
    engine_.cancel();
    if (future_.valid()) {
        future_.get();
    }
}

void app_model::is_rendering() const { engine_.is_running(); }

//  SAVE
//  //////////////////////////////////////////////////////////////////
//  TODO
//  If currently_open_file_ is blank/not-a-.way then fire a callback
//  asking
//  for a save location.
//  Add .way to the file if it is not specified.
//  Otherwise just overwrite the existing file.
void app_model::save() const {}

//  TODO
//  Add .way to the file if it is not specified.
//  Write the project to the specifed path, and set
//  currently_open_file_.
void app_model::save_as(std::string name) const {}

//  DATA
//  //////////////////////////////////////////////////////////////////
//  TODO scene materials

const source_model& app_model::get_source(size_t index) const {
    return sources_[index];
}
source_model& app_model::get_source(size_t index) { return sources_[index]; }

void app_model::add_source(size_t index) {
    sources_.add(index, aabb_);
    sources_[index].connect_on_change([&](auto&) { on_change_(*this); });
}

void app_model::remove_source(size_t index) {
    if (1 < sources_.size()) {
        sources_.remove(index);
    }
}

const receiver_model& app_model::get_receiver(size_t index) const {
    return receivers_[index];
}
receiver_model& app_model::get_receiver(size_t index) {
    return receivers_[index];
}

void app_model::add_receiver(size_t index) {
    receivers_.add(index, aabb_);
    receivers_[index].connect_on_change([&](auto&) { on_change_(*this); });
}

void app_model::remove_receiver(size_t index) {
    if (1 < receivers_.size()) {
        receivers_.remove(index);
    }
}

engine_state_changed::connection app_model::connect_engine_state(
        engine_state_changed::callback_type t) {
    return engine_.add_engine_state_changed_callback(std::move(t));
}

waveguide_node_positions_changed::connection app_model::connect_node_positions(
        waveguide_node_positions_changed::callback_type t) {
    return engine_.add_waveguide_node_positions_changed_callback(std::move(t));
}

waveguide_node_pressures_changed::connection app_model::connect_node_pressures(
        waveguide_node_pressures_changed::callback_type t) {
    return engine_.add_waveguide_node_pressures_changed_callback(std::move(t));
}

raytracer_reflections_generated::connection app_model::connect_reflections(
        raytracer_reflections_generated::callback_type t) {
    return engine_.add_raytracer_reflections_generated_callback(std::move(t));
}

app_model::encountered_error::connection app_model::connect_error_handler(
        encountered_error::callback_type t) {
    return engine_.add_encountered_error_callback(std::move(t));
}

app_model::on_change::connection app_model::connect_on_change(
        on_change::callback_type t) {
    return on_change_.connect(std::move(t));
}

}  // namespace combined
}  // namespace wayverb
