#pragma once

#include "model.hpp"

#include <array>

namespace model {

template <typename T>
class NestedValueWrapper : public ModelValue<T> {
public:
    NestedValueWrapper(ModelMember *owner, T &t)
            : ModelValue<T>(owner)
            , t(&t) {
    }

    T get() const override {
        return *t;
    }

protected:
    T *t;
};

template <typename T>
class ValueWrapper : public NestedValueWrapper<T> {
public:
    using NestedValueWrapper<T>::NestedValueWrapper;

    void reseat(T &u) override {
        this->t = &u;
    }
    void set(const T &u, bool do_notify = true) override {
        *(this->t) = u;
        if (do_notify) {
            this->broadcast();
        }
    }
};

template <>
class ValueWrapper<bool> : public NestedValueWrapper<bool> {
public:
    using NestedValueWrapper<bool>::NestedValueWrapper;

    void reseat(bool &u) override {
        this->t = &u;
    }
    void set(const bool &u, bool do_notify = true) override {
        *(this->t) = u;
        if (do_notify) {
            this->broadcast();
        }
    }

    void toggle(bool do_notify = true) {
        set(!(*(this->t)), do_notify);
    }
};

template <typename Struct>
class StructAccessor {
public:
    StructAccessor() = default;
    StructAccessor(const StructAccessor &) = default;
    StructAccessor &operator=(const StructAccessor &) = default;
    StructAccessor(StructAccessor &&) noexcept = default;
    StructAccessor &operator=(StructAccessor &&) noexcept = default;
    virtual ~StructAccessor() noexcept = default;

    virtual void reseat_from_struct(Struct &s) = 0;
    virtual void set_from_struct(const Struct &s, bool do_notify) = 0;
};

template <typename Struct, typename Field>
class NestedValueAccessor : public StructAccessor<Struct>,
                            public ValueWrapper<Field> {
public:
    using Accessor = std::function<Field &(Struct &)>;
    using ConstAccessor = std::function<const Field &(const Struct &)>;
    NestedValueAccessor(ModelMember *owner,
                        Struct &s,
                        Accessor accessor,
                        ConstAccessor const_accessor)
            : ValueWrapper<Field>(owner, accessor(s))
            , accessor(accessor)
            , const_accessor(const_accessor) {
    }

    void reseat_from_struct(Struct &s) override {
        this->reseat(accessor(s));
    }
    void set_from_struct(const Struct &s, bool do_notify = true) override {
        this->set(const_accessor(s), do_notify);
    }

private:
    Accessor accessor;
    ConstAccessor const_accessor;
};

#define RENAMED_FIELD_DEFINITION(member, name)                             \
    NestedValueAccessor<                                                   \
            wrapped,                                                       \
            std::decay_t<decltype(std::declval<wrapped>().member)>>        \
            name{this,                                                     \
                 *t,                                                       \
                 [](auto &i) -> decltype(i.member) & { return i.member; }, \
                 [](const auto &i) -> const decltype(i.member) & {         \
                     return i.member;                                      \
                 }};

#define MODEL_FIELD_DEFINITION(name) RENAMED_FIELD_DEFINITION(name, name)

template <typename T, size_t members>
class StructWrapper : public NestedValueWrapper<T> {
public:
    using struct_wrapper = StructWrapper;
    using wrapped = T;
    using member_array = std::array<StructAccessor<T> *, members>;
    using NestedValueWrapper<T>::NestedValueWrapper;

    void reseat(T &u) override {
        for (auto &i : this->get_members()) {
            i->reseat_from_struct(u);
        }
    }
    void set(const T &u, bool do_notify = true) override {
        for (auto &i : this->get_members()) {
            i->set_from_struct(u, do_notify);
        }
    }

    virtual member_array get_members() = 0;
};

}  // namespace model
