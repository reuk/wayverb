#pragma once

#include "common/boundaries.h"
#include "common/callbacks.h"
#include "common/cl_common.h"

#include "raytracer/raytracer.h"
#include "waveguide/waveguide.h"

/*
Not sure how to structure this
Two main usecases:
    * offline rendering
        * construct
        * maybe do some set-up
        * run the whole thing, with some way of monitoring progress
        * fetch the result
    * visualised
        * construct
        * maybe do some set-up
        * every step, update display
            * hopefully directly using a GL native buffer

            * every frame, tell the engine that it is allowed to progress to
              the next step
            * every time the engine finishes a step, it must wait for
              confirmation from the frame thread - spinlock? semaphore?
            * the renderer keeps a reference to the same buffer as the engine,
              so I don't need to pass arrays around

            * renderer can run all in one go (as a std::future?)

        * fetch the result

Differences:
    * single 'run' call vs multiple run_step calls
    * type of buffer to be used - don't want to link against OpenGL unless it's
      actually being used
    * result fetch method

*/

/// The Wayverb engine
template<BufferType buffer_type>
class WayverbEngine {
public:
    enum class State {
        starting_raytracer,
        running_raytracer,
        finishing_raytracer,
        starting_waveguide,
        running_waveguide,
        finishing_waveguide,
        postprocessing,
    };

    WayverbEngine(ComputeContext& compute_context,
                  const SceneData& scene_data,
                  const Vec3f& source,
                  const Vec3f& mic,
                  float waveguide_sample_rate,
                  int rays,
                  int impulses,
                  float output_sample_rate);

    struct Intermediate {};

    using StateCallback = GenericArgumentsCallback<State, double>;

    Intermediate run(std::atomic_bool& keep_going,
                     const StateCallback& callback);
    std::vector<std::vector<float>> attenuate(const Intermediate& i,
                                              //  other args or whatever
                                              const StateCallback& callback);

    template <typename Callback = StateCallback>
    auto run(std::atomic_bool& keep_going,
             const Callback& callback = Callback()) {
        return run(keep_going,
                   static_cast<const StateCallback&>(make_adapter(callback)));
    }

    template <typename Callback = StateCallback>
    auto attenuate(const Intermediate& i,
                   //  other args or whatever
                   const Callback& callback = Callback()) {
        return attenuate(
            i,
            //  other args or whatever
            static_cast<const StateCallback&>(make_adapter(callback)));
    }

private:
    template <typename Callback>
    auto make_adapter(const Callback& callback) {
        return GenericCallbackAdapter<Callback, State, double>(callback);
    }

    SceneData scene_data;
    Raytracer raytracer;
    RectangularWaveguide<buffer_type> waveguide;

    Vec3f source;
    Vec3f mic;
    float waveguide_sample_rate;
    int rays;
    int impulses;
    //float output_sample_rate;

    size_t source_index;
    size_t mic_index;
};
