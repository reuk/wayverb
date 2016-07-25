#include "raytracer/attenuator.h"

#include "common/hrtf.h"
#include "common/conversions.h"

namespace raytracer {
namespace attenuator {

hrtf::hrtf(const cl::Context& context,
                               const cl::Device& device)
        : queue(context, device)
        , kernel(attenuator_program(context, device).get_hrtf_kernel())
        , cl_hrtf(context, CL_MEM_READ_WRITE, sizeof(VolumeType) * 360 * 180) {}

aligned::vector<AttenuatedImpulse> hrtf::process(
        const results::selected& results,
        const glm::vec3& direction,
        const glm::vec3& up,
        const glm::vec3& position,
        HrtfChannel channel) {
    auto channel_index = channel == HrtfChannel::left ? 0 : 1;
    //  muck around with the table format
    aligned::vector<VolumeType> hrtf_channel_data(360 * 180);
    auto offset = 0;
    for (const auto& i : get_hrtf_data()[channel_index]) {
        proc::copy(i, hrtf_channel_data.begin() + offset);
        offset += i.size();
    }

    //  copy hrtf table to buffer
    cl::copy(queue, begin(hrtf_channel_data), end(hrtf_channel_data), cl_hrtf);

    auto impulses = results.get_impulses();

    //  set up buffers
    auto context = queue.getInfo<CL_QUEUE_CONTEXT>();
    cl::Buffer cl_in(
            context, CL_MEM_READ_WRITE, impulses.size() * sizeof(Impulse));
    cl::Buffer cl_out(context,
                      CL_MEM_READ_WRITE,
                      impulses.size() * sizeof(AttenuatedImpulse));

    //  copy input to buffer
    cl::copy(queue, impulses.begin(), impulses.end(), cl_in);

    //  run kernel
    kernel(cl::EnqueueArgs(queue, cl::NDRange(impulses.size())),
           to_cl_float3(position),
           cl_in,
           cl_out,
           cl_hrtf,
           to_cl_float3(direction),
           to_cl_float3(up),
           channel_index);

    //  create output storage
    aligned::vector<AttenuatedImpulse> ret(impulses.size());

    //  copy to output
    cl::copy(queue, cl_out, ret.begin(), ret.end());

    return ret;
}

const std::array<std::array<std::array<cl_float8, 180>, 360>, 2>&
hrtf::get_hrtf_data() const {
    return HrtfData::HRTF_DATA;
}

microphone::microphone(const cl::Context& context,
                                           const cl::Device& device)
        : queue(context, device)
        , kernel(attenuator_program(context, device).get_microphone_kernel()) {}

aligned::vector<AttenuatedImpulse> microphone::process(
        const results::selected& results,
        const glm::vec3& pointing,
        float shape,
        const glm::vec3& position) {
    auto impulses = results.get_impulses();
    //  init buffers
    auto context = queue.getInfo<CL_QUEUE_CONTEXT>();
    cl::Buffer cl_in(
            context, CL_MEM_READ_WRITE, impulses.size() * sizeof(Impulse));

    cl::Buffer cl_out(context,
                      CL_MEM_READ_WRITE,
                      impulses.size() * sizeof(AttenuatedImpulse));
    aligned::vector<AttenuatedImpulse> zero(
            impulses.size(), AttenuatedImpulse{{{0, 0, 0, 0, 0, 0, 0, 0}}, 0});
    cl::copy(queue, zero.begin(), zero.end(), cl_out);

    //  copy input data to buffer
    cl::copy(queue, impulses.begin(), impulses.end(), cl_in);

    //  run kernel
    kernel(cl::EnqueueArgs(queue, cl::NDRange(impulses.size())),
           to_cl_float3(position),
           cl_in,
           cl_out,
           Speaker{to_cl_float3(pointing), shape});

    //  create output location
    aligned::vector<AttenuatedImpulse> ret(impulses.size());

    //  copy from buffer to output
    cl::copy(queue, cl_out, ret.begin(), ret.end());

    return ret;
}

}  // namespace attenuator
}  // namespace raytracer
