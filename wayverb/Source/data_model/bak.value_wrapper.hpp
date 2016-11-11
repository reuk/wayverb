#pragma once

#include "model.hpp"

namespace model {

template <typename T>
class ValueWrapper : public ModelValue<T> {
public:
    ValueWrapper(ModelMember *owner, const T &t)
            : ModelValue<T>(owner)
            , t(t) {
    }

    T get() const override {
        return t;
    }

    void set(const T &u, bool do_notify = true) override {
        this->t = u;
        if (do_notify) {
            this->broadcast();
        }
    }

protected:
    T t;
};

}  // namespace model