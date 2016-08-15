#pragma once

namespace cl_sources {

constexpr const char* cat{R"(
#ifndef CAT_DEFINITION__
#define CAT_DEFINITION__
#define CAT(a, b) PRIMITIVE_CAT(a, b)
#define PRIMITIVE_CAT(a, b) a##b
#endif
)"};

}  // namespace cl_sources
