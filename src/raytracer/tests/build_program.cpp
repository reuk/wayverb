#include "raytracer/program.h"

#include "gtest/gtest.h"

TEST(build_program, raytracer) {
    for (const auto& dev : {core::device_type::cpu, core::device_type::gpu}) {
        const core::compute_context cc{dev};
        ASSERT_NO_THROW(raytracer::program{cc});
    }
}
