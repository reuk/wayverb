#pragma once

#include "value_wrapper.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace model {

template <typename T, typename Allocator>
class ValueWrapper<std::vector<T, Allocator>>
        : public ModelValue<std::vector<T, Allocator>> {
    using model_type = std::vector<std::unique_ptr<ValueWrapper<T>>>;

    model_type generate_t(const std::vector<T, Allocator>& t) {
        model_type ret;
        ret.reserve(t.size());
        for (const auto& i : t) {
            ret.push_back(std::make_unique<ValueWrapper<T>>(this, i));
        }
        return ret;
    }

public:
    ValueWrapper(ModelMember* owner, const std::vector<T, Allocator>& t)
            : ModelValue<std::vector<T, Allocator>>(owner)
            , t(generate_t(t)) {
    }

    std::vector<T, Allocator> get() const override {
        std::vector<T, Allocator> ret;
        ret.reserve(t.size());
        for (const auto& i : t) {
            ret.push_back(i->get());
        }
        return ret;
    }

    void set(const std::vector<T, Allocator>& u, bool do_notify = true) override {
        t = generate_t(u);
        if (do_notify) {
            this->broadcast();
        }
    }

    ValueWrapper<T>& at(size_t pos) {
        return *t.at(pos);
    }
    const ValueWrapper<T>& at(size_t pos) const {
        return *t.at(pos);
    }

    ValueWrapper<T>& operator[](size_t pos) {
        return *t[pos];
    }
    const ValueWrapper<T>& operator[](size_t pos) const {
        return *t[pos];
    }

    ValueWrapper<T>& front() {
        return *t.front();
    }
    const ValueWrapper<T>& front() const {
        return *t.front();
    }

    ValueWrapper<T>& back() {
        return *t.back();
    }
    const ValueWrapper<T>& back() const {
        return *t.back();
    }

    bool empty() const {
        return t.empty();
    }
    size_t size() const {
        return t.size();
    }

    auto begin() {
        return t.begin();
    }

    auto begin() const {
        return t.begin();
    }

    auto cbegin() const {
        return t.cbegin();
    }

    auto end() {
        return t.end();
    }

    auto end() const {
        return t.end();
    }

    auto cend() const {
        return t.cend();
    }

    //  modifiers
    void clear(bool do_notify = true) {
        t.clear();
        if (do_notify) {
            this->broadcast();
        }
    }

    void insert(size_t pos, const T& i, bool do_notify = true) {
        t.insert(t.begin() + pos, std::make_unique<ValueWrapper<T>>(this, i));
        if (do_notify) {
            this->broadcast();
        }
    }

    void erase(size_t pos, bool do_notify = true) {
        t.erase(t.begin() + pos);
        if (do_notify) {
            this->broadcast();
        }
    }

    void push_back(const T& i, bool do_notify = true) {
        t.push_back(std::make_unique<ValueWrapper<T>>(this, i));
        if (do_notify) {
            this->broadcast();
        }
    }

    void pop_back(bool do_notify = true) {
        t.pop_back();
        if (do_notify) {
            this->broadcast();
        }
    }

private:
    model_type t;
};
}  // namespace model
