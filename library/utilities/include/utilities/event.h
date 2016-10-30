#pragma once

#include "utilities/final_act.h"

#include <functional>
#include <unordered_map>

template <typename... Ts>
class event final {
    class impl;

public:
    event()
            : pimpl_{std::make_unique<impl>()} {}

    using key_type = size_t;
    using callback_type = std::function<void(Ts&&... ts)>;

    auto add(callback_type callback) {
        return pimpl_->add(std::move(callback));
    }

    class disconnector final {
    public:
        constexpr disconnector(impl& impl, key_type key)
                : impl_{&impl}
                , key_{key} {}

        void operator()() const noexcept { impl_->remove(key_); }

    private:
        impl* impl_;
        key_type key_;
    };

    using scoped_connector = final_act<disconnector>;

    /// Returns a connection object which will automatically remove the
    /// connection when it goes out of scope.
    /// Of course, to avoid dangling references, the scoped connection must
    /// be destroyed before the signal itself.
    auto add_scoped(callback_type callback) {
        return make_final_act(disconnector{*pimpl_, add(std::move(callback))});
    }

    void remove(key_type key) { pimpl_.remove(key); }
    void remove_all() { pimpl_->remove_all(); }

    void operator()(Ts&&... ts) const {
        pimpl_->operator()(std::forward<Ts>(ts)...);
    }

private:
    class impl final {
    public:
        impl() = default;

        impl(const impl&) = delete;
        impl(impl&&) noexcept = delete;

        impl& operator=(const impl&) = delete;
        impl& operator=(impl&&) noexcept = delete;

        auto add(callback_type callback) {
            slots_.insert(std::make_pair(++current_key_, std::move(callback)));
            return current_key_;
        }

        void remove(key_type key) { slots_.erase(key); }
        void remove_all() { slots_.clear(); }

        void operator()(Ts&&... ts) const {
            for (const auto& slot : slots_) {
                slot.second(std::forward<Ts>(ts)...);
            }
        }

    private:
        key_type current_key_{0};
        std::unordered_map<key_type, callback_type> slots_;
    };

    std::unique_ptr<impl> pimpl_;
};
