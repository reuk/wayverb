#include "ostream_overloads.h"

std::ostream& operator<<(std::ostream& os, const cl_float3& f) {
    Bracketer bracketer(os);
    return to_stream(os, f.s[0], "  ", f.s[1], "  ", f.s[2], "  ");
}
