#pragma once

#include "Model.h"
#include "collection.h"

namespace model {

struct ImpulseRouting {
    std::string name{""};
    int channel{-1};  // -1 for none
};

template <>
class model::ValueWrapper<ImpulseRouting>
        : public StructWrapper<ImpulseRouting, 2> {
public:
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
        return {{&name, &channel}};
    }
    MODEL_FIELD_DEFINITION(name);
    MODEL_FIELD_DEFINITION(channel);
};

struct CarrierRouting {
    using my_bool = int8_t;
    std::string name{""};
    std::vector<my_bool> channel;
};

template <>
class model::ValueWrapper<CarrierRouting>
        : public StructWrapper<CarrierRouting, 2> {
public:
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
        return {{&name, &channel}};
    }
    MODEL_FIELD_DEFINITION(name);
    MODEL_FIELD_DEFINITION(channel);
};
}  // namespace model