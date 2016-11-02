#include "waveguide/program.h"

#include "gtest/gtest.h"

using namespace wayverb::waveguide;
using namespace wayverb::core;

TEST(build_program, waveguide) {
    for (const auto& dev : {device_type::cpu, device_type::gpu}) {
        const compute_context cc{dev};
        ASSERT_NO_THROW(program{cc});
    }
}
