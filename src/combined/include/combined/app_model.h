#pragma once

#include "combined/threaded_engine.h"

#include <future>

namespace wayverb {
namespace combined {

template <typename T>
class vector_model final {
public:
    vector_model() = default;

    vector_model(const vector_model&) = delete;
    vector_model(vector_model&&) noexcept = delete;

    vector_model& operator=(const vector_model&) = delete;
    vector_model& operator=(vector_model&&) noexcept = delete;

    const auto& operator[](size_t index) const { return *data_[index]; }
    auto& operator[](size_t index) { return *data_[index]; }

    template <typename... Ts>
    void add(size_t index, Ts&&... ts) {
        auto ret = std::make_unique<T>(std::forward<Ts>(ts)...);
        data_.insert(data_.begin() + index, std::move(ret));
        on_change_(*this);
    }

    void remove(size_t index) {
        data_.erase(data_.begin() + index);
        on_change_(*this);
    }

    auto size() const { return data_.size(); }
    auto empty() const { return data_.empty(); }

    void clear() {
        data_.clear();
        on_change_(*this);
    }

    auto get_raw() const {
        return util::map_to_vector(begin(data_), end(data_), [](const auto& i) {
            return i->get_raw();
        });
    }

    template <typename U>
    auto connect_on_change(U&& t) {
        return on_change_.connect(std::forward<U>(t));
    }

private:
    std::vector<std::unique_ptr<T>> data_;

    util::event<vector_model&> on_change_;
};

////////////////////////////////////////////////////////////////////////////////

class source_model final {
public:
    source_model(core::geo::box bounds);

    source_model(const source_model&) = delete;
    source_model(source_model&&) noexcept = delete;

    source_model& operator=(const source_model&) = delete;
    source_model& operator=(source_model&&) noexcept = delete;

    void set_name(std::string name);
    std::string get_name() const;

    void set_position(const glm::vec3& position);
    glm::vec3 get_position() const;

    source_info get_raw() const;

    using on_change = util::event<source_model&>;
    on_change::connection connect_on_change(on_change::callback_type t);

private:
    core::geo::box bounds_;

    source_info data_;

    on_change on_change_;
};

////////////////////////////////////////////////////////////////////////////////

class microphone_model final {
public:
    microphone_model() = default;

    microphone_model(const microphone_model&) = delete;
    microphone_model(microphone_model&&) noexcept = delete;

    microphone_model& operator=(const microphone_model&) = delete;
    microphone_model& operator=(microphone_model&&) noexcept = delete;

    void set_orientation(float azimuth, float elevation);
    void set_shape(double shape);

    core::attenuator::microphone get_raw() const;

    using on_change = util::event<microphone_model&>;
    on_change::connection connect_on_change(on_change::callback_type t);

private:
    core::attenuator::microphone microphone_;

    on_change on_change_;
};

class hrtf_model final {
public:
    hrtf_model() = default;

    hrtf_model(const hrtf_model&) = delete;
    hrtf_model(hrtf_model&&) noexcept = delete;

    hrtf_model& operator=(const hrtf_model&) = delete;
    hrtf_model& operator=(hrtf_model&&) noexcept = delete;

    void set_orientation(float azimuth, float elevation);
    void set_channel(core::attenuator::hrtf::channel channel);

    core::attenuator::hrtf get_raw() const;

    using on_change = util::event<hrtf_model&>;
    on_change::connection connect_on_change(on_change::callback_type t);

private:
    core::attenuator::hrtf hrtf_;

    on_change on_change_;
};

////////////////////////////////////////////////////////////////////////////////

class capsule_model final {
public:
    capsule_model();

    capsule_model(const capsule_model&) = delete;
    capsule_model(capsule_model&&) noexcept = delete;

    capsule_model& operator=(const capsule_model&) = delete;
    capsule_model& operator=(capsule_model&&) noexcept = delete;

    enum class type { microphone, hrtf };

    void set_type(type type);
    void set_name(std::string name);

    using on_change = util::event<capsule_model&>;
    on_change::connection connect_on_change(on_change::callback_type t);

    microphone_model microphone;
    hrtf_model hrtf;

private:
    type type_ = type::microphone;
    std::string name_ = "new capsule";

    on_change on_change_;
};

////////////////////////////////////////////////////////////////////////////////

class receiver_model final {
public:
    receiver_model(core::geo::box bounds);

    receiver_model(const receiver_model&) = delete;
    receiver_model(receiver_model&&) noexcept = delete;

    receiver_model& operator=(const receiver_model&) = delete;
    receiver_model& operator=(receiver_model&&) noexcept = delete;

    void set_name(std::string name);
    void set_position(const glm::vec3& position);
    void set_orientation(float azimuth, float elevation);

    const capsule_model& get_capsule(size_t index) const;
    capsule_model& get_capsule(size_t index);
    void add_capsule(size_t index);
    void remove_capsule(size_t index);

    using on_change = util::event<receiver_model&>;
    on_change::connection connect_on_change(on_change::callback_type t);

private:
    core::geo::box bounds_;

    std::string name_;
    glm::vec3 position_;
    core::orientable orientation_;
    vector_model<capsule_model> capsules_;

    on_change on_change_;
};

////////////////////////////////////////////////////////////////////////////////

class raytracer_model final {
public:
    raytracer_model() = default;

    raytracer_model(const raytracer_model&) = delete;
    raytracer_model(raytracer_model&&) noexcept = delete;

    raytracer_model& operator=(const raytracer_model&) = delete;
    raytracer_model& operator=(raytracer_model&&) noexcept = delete;

    void set_rays(size_t rays);
    void set_max_img_src_order(size_t max);

    using on_change = util::event<raytracer_model&>;
    on_change::connection connect_on_change(on_change::callback_type t);

private:
    size_t rays_ = 10000;
    size_t max_img_src_order_ = 4;

    on_change on_change_;
};

class single_band_waveguide_model final {
public:
    single_band_waveguide_model() = default;

    single_band_waveguide_model(const single_band_waveguide_model&) = delete;
    single_band_waveguide_model(single_band_waveguide_model&&) = delete;

    single_band_waveguide_model& operator=(const single_band_waveguide_model&) =
            delete;
    single_band_waveguide_model& operator=(single_band_waveguide_model&&) =
            delete;

    void set_cutoff(double cutoff);
    void set_usable_portion(double usable);

    using on_change = util::event<single_band_waveguide_model&>;
    on_change::connection connect_on_change(on_change::callback_type t);

private:
    double cutoff_ = 500;
    double usable_portion_ = 0.6;

    on_change on_change_;
};

class multiple_band_waveguide_model final {
public:
    multiple_band_waveguide_model() = default;

    multiple_band_waveguide_model(const multiple_band_waveguide_model&) =
            delete;
    multiple_band_waveguide_model(multiple_band_waveguide_model&&) noexcept =
            delete;

    multiple_band_waveguide_model& operator=(
            const multiple_band_waveguide_model&) = delete;
    multiple_band_waveguide_model& operator=(
            multiple_band_waveguide_model&&) noexcept = delete;

    void set_bands(size_t bands);
    void set_cutoff(double cutoff);
    void set_usable_portion(double usable);

    using on_change = util::event<multiple_band_waveguide_model&>;
    on_change::connection connect_on_change(on_change::callback_type t);

private:
    const frequency_domain::edges_and_width_factor<9> band_params_ =
            hrtf_data::hrtf_band_params_hz();

    void maintain_valid_cutoff();

    size_t bands_ = 2;
    double cutoff_ = 500;
    double usable_portion_ = 0.6;

    on_change on_change_;
};

class waveguide_model final {
public:
    waveguide_model();

    waveguide_model(const waveguide_model&) = delete;
    waveguide_model(waveguide_model&&) noexcept = delete;

    waveguide_model& operator=(const waveguide_model&) = delete;
    waveguide_model& operator=(waveguide_model&&) noexcept = delete;

    enum class type { single_band, multiple_band };

    void set_type(type type);

    using on_change = util::event<waveguide_model&>;
    on_change::connection connect_on_change(on_change::callback_type t);

    single_band_waveguide_model single_band;
    multiple_band_waveguide_model multiple_band;

private:
    type type_;

    on_change on_change_;
};

////////////////////////////////////////////////////////////////////////////////

class output_model final {
public:
    output_model() = default;

    output_model(const output_model&) = delete;
    output_model(output_model&&) noexcept = delete;

    output_model& operator=(const output_model&) = delete;
    output_model& operator=(output_model&&) noexcept = delete;

    void set_output_folder(std::string output_folder);
    void set_name(std::string name);
    void set_sample_rate(double sr);

    enum class bit_depth { bd16, bd24 };

    void set_bit_depth(bit_depth bit_depth);

    using on_change = util::event<output_model&>;
    on_change::connection connect_on_change(on_change::callback_type t);

private:
    std::string output_folder_ = ".";
    std::string name_ = "sig";
    double sample_rate_ = 44100.0;
    bit_depth bit_depth_ = bit_depth::bd16;

    on_change on_change_;
};

////////////////////////////////////////////////////////////////////////////////

class app_model final {
public:
    //  SPECIAL MEMBERS  ///////////////////////////////////////////////////////
    //  TODO
    //  If the file is a .way then open scene + config.
    //  Otherwise, attempt to read as a normal 3D file and use defaults for
    //  everything else.
    app_model(const std::string& name);

    app_model(const app_model&) = delete;
    app_model(app_model&&) noexcept = delete;

    app_model& operator=(const app_model&) = delete;
    app_model& operator=(app_model&&) noexcept = delete;

    ~app_model() noexcept;

    //  RENDERING  /////////////////////////////////////////////////////////////
    void start_render();
    void stop_render();
    void is_rendering() const;

    //  SAVE  //////////////////////////////////////////////////////////////////
    //  TODO
    //  If currently_open_file_ is blank/not-a-.way then fire a callback asking
    //  for a save location.
    //  Add .way to the file if it is not specified.
    //  Otherwise just overwrite the existing file.
    void save() const;

    //  TODO
    //  Add .way to the file if it is not specified.
    //  Write the project to the specifed path, and set currently_open_file_.
    void save_as(std::string name) const;

    //  DATA  //////////////////////////////////////////////////////////////////
    //  TODO scene materials

    const source_model& get_source(size_t index) const;
    source_model& get_source(size_t index);
    void add_source(size_t index);
    void remove_source(size_t index);

    const receiver_model& get_receiver(size_t index) const;
    receiver_model& get_receiver(size_t index);
    void add_receiver(size_t index);
    void remove_receiver(size_t index);

    raytracer_model raytracer;
    waveguide_model waveguide;
    output_model output;

    //  CALLBACKS  /////////////////////////////////////////////////////////////

    engine_state_changed::connection connect_engine_state(
            engine_state_changed::callback_type t);

    waveguide_node_positions_changed::connection connect_node_positions(
            waveguide_node_positions_changed::callback_type t);

    waveguide_node_pressures_changed::connection connect_node_pressures(
            waveguide_node_pressures_changed::callback_type t);

    raytracer_reflections_generated::connection connect_reflections(
            raytracer_reflections_generated::callback_type t);

    using encountered_error = complete_engine::encountered_error;
    encountered_error::connection connect_error_handler(
            encountered_error::callback_type t);

    using on_change = util::event<app_model&>;
    on_change::connection connect_on_change(on_change::callback_type t);

private:
    std::string currently_open_file_;

    const core::scene_data_loader scene_;
    const core::geo::box aabb_;

    vector_model<source_model> sources_;
    vector_model<receiver_model> receivers_;

    complete_engine engine_;
    std::future<void> future_;

    on_change on_change_;
};

}  // namespace combined
}  // namespace wayverb
