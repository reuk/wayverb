#pragma once

#include "Model.h"

namespace model {

template <typename T>
class NestedValueWrapper : public ModelValue<T> {
public:
    NestedValueWrapper(ModelMember* owner, T& t)
            : ModelValue<T>(owner)
            , t(&t) {
    }

    virtual void reseat(T& u) {
        t = &u;
    }

protected:
    T get_value() const override {
        return *t;
    }

    T* t;
};

template <typename T>
class ValueWrapper : public NestedValueWrapper<T> {
public:
    using NestedValueWrapper<T>::NestedValueWrapper;

protected:
    void set_value(const T& u, bool do_notify = true) override {
        *(this->t) = u;
        this->notify(do_notify);
    }
};

}  // namespace model