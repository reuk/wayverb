#pragma once

#include "core/model/orientable.h"

namespace model {

struct microphone final {
    orientable orientable{};
    float shape{0};
};

}  // namespace model
