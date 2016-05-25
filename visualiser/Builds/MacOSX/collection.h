#pragma once

#include "value_wrapper.h"

#include <cassert>
#include <vector>

namespace model {

template <typename It, typename... Ts, typename Func>
void for_each(Func&& f, It first, It last, Ts&&... ts) {
    while (first != last) {
        f(*first++, (*ts++)...);
    }
}

template <typename T>
class ValueWrapper<std::vector<T>> : public NestedValueWrapper<std::vector<T>> {
public:
    ValueWrapper(ModelMember* parent, std::vector<T>& t)
            : NestedValueWrapper<std::vector<T>>(parent, t)
            , wrappers(t.size()) {
        this->reseat(t);
    }

    void set(const std::vector<T>& u, bool do_notify = true) override {
        std::lock_guard<std::mutex> lck(this->mut);
        *(this->t) = u;
        reseat_and_notify(do_notify);
    }

    ValueWrapper<T>& at(size_t pos) {
        std::lock_guard<std::mutex> lck(this->mut);
        return *wrappers.at(pos);
    }
    const ValueWrapper<T>& at(size_t pos) const {
        std::lock_guard<std::mutex> lck(this->mut);
        return *wrappers.at(pos);
    }

    ValueWrapper<T>& operator[](size_t pos) {
        std::lock_guard<std::mutex> lck(this->mut);
        return *wrappers[pos];
    }
    const ValueWrapper<T>& operator[](size_t pos) const {
        std::lock_guard<std::mutex> lck(this->mut);
        return *wrappers[pos];
    }

    ValueWrapper<T>& front() {
        std::lock_guard<std::mutex> lck(this->mut);
        return *wrappers.front();
    }
    const ValueWrapper<T>& front() const {
        std::lock_guard<std::mutex> lck(this->mut);
        return *wrappers.front();
    }

    ValueWrapper<T>& back() {
        std::lock_guard<std::mutex> lck(this->mut);
        return *wrappers.back();
    }
    const ValueWrapper<T>& back() const {
        std::lock_guard<std::mutex> lck(this->mut);
        return *wrappers.back();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lck(this->mut);
        return wrappers.empty();
    }
    size_t size() const {
        std::lock_guard<std::mutex> lck(this->mut);
        return wrappers.size();
    }

    auto begin() {
        std::lock_guard<std::mutex> lck(this->mut);
        return wrappers.begin();
    }

    auto begin() const {
        std::lock_guard<std::mutex> lck(this->mut);
        return wrappers.begin();
    }

    auto cbegin() const {
        std::lock_guard<std::mutex> lck(this->mut);
        return wrappers.cbegin();
    }

    auto end() {
        std::lock_guard<std::mutex> lck(this->mut);
        return wrappers.end();
    }

    auto end() const {
        std::lock_guard<std::mutex> lck(this->mut);
        return wrappers.end();
    }

    auto cend() const {
        std::lock_guard<std::mutex> lck(this->mut);
        return wrappers.cend();
    }

    //  modifiers
    void clear(bool do_notify = true) {
        std::lock_guard<std::mutex> lck(this->mut);
        wrappers.clear();
        this->t->clear();
        reseat_and_notify(do_notify);
    }

    void insert(size_t pos, const T& value, bool do_notify = true) {
        std::lock_guard<std::mutex> lck(this->mut);
        this->t->insert(this->t->begin() + pos, value);
        wrappers.insert(wrappers.begin() + pos, nullptr);
        reseat_and_notify(do_notify);
    }

    void erase(size_t pos, bool do_notify = true) {
        std::lock_guard<std::mutex> lck(this->mut);
        this->t->erase(this->t->begin() + pos);
        wrappers.erase(wrappers.begin() + pos);
        reseat_and_notify(do_notify);
    }

    void push_back(const T& value, bool do_notify = true) {
        std::lock_guard<std::mutex> lck(this->mut);
        this->t->push_back(value);
        wrappers.push_back(nullptr);
        reseat_and_notify(do_notify);
    }

    void pop_back(bool do_notify = true) {
        std::lock_guard<std::mutex> lck(this->mut);
        this->t->pop_back();
        wrappers.pop_back();
        reseat_and_notify(do_notify);
    }

    void resize(size_t num, const T& value = T(), bool do_notify = true) {
        std::lock_guard<std::mutex> lck(this->mut);
        this->t->resize(num, value);
        wrappers.resize(num, nullptr);
        reseat_and_notify(do_notify);
    }

    void reseat(std::vector<T>& u) override {
        std::lock_guard<std::mutex> lck(this->mut);
        this->reseat_impl(u);
    }

private:
    void reseat_impl(std::vector<T>& u) {
        assert(u.size() == wrappers.size());
        for_each(
            [this](auto& i, auto& j) {
                if (j) {
                    j->reseat(i);
                } else {
                    j = std::make_unique<ValueWrapper<T>>(this, i);
                }
            },
            u.begin(),
            u.end(),
            wrappers.begin());
    }

    void reseat_and_notify(bool do_notify) {
        this->reseat_impl(*(this->t));
        this->notify(do_notify);
    }

    std::vector<std::unique_ptr<ValueWrapper<T>>> wrappers;
};
}  // namespace model
