#pragma once

#include "core/model/orientable.h"

namespace wayverb {
namespace core {
namespace model {

struct microphone final {
    orientable orientable{};
    float shape{0};
};

}  // namespace model
}  // namespace core
}  // namespace wayverb
