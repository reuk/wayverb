#include "utilities/string_builder.h"

namespace util {

Bracketer::Bracketer(std::ostream& os, const char* open, const char* closed)
        : os(os)
        , closed(closed) {
    os << open << "  ";
}

Bracketer::~Bracketer() {
    os << closed << "  ";
}

}  // namespace util
