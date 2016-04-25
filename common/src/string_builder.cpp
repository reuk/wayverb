#include "common/string_builder.h"

Bracketer::Bracketer(std::ostream& os, const char* open, const char* closed)
        : os(os)
        , closed(closed) {
    os << open << "  ";
}

Bracketer::~Bracketer() {
    os << closed << "  ";
}
