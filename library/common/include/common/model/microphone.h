#pragma once

#include "common/model/orientable.h"

namespace model {

struct microphone final {
    orientable orientable{};
    float shape{0};
};

}  // namespace model
