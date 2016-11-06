#pragma once

#include "utilities/final_act.h"

#include <functional>
#include <unordered_map>

namespace util {

template <typename... Ts>
class event final {
    class impl;

public:
    event()
            : pimpl_{std::make_shared<impl>()} {}

    using key_type = size_t;
    using callback_type = std::function<void(Ts... ts)>;

    auto add(callback_type callback) {
        return pimpl_->add(std::move(callback));
    }

    class disconnector final {
    public:
        disconnector() = default;
        disconnector(std::shared_ptr<impl> pimpl, key_type key)
                : pimpl_{pimpl}
                , key_{key} {}

        void operator()() const noexcept { pimpl_->remove(key_); }

    private:
        std::shared_ptr<impl> pimpl_;
        key_type key_;
    };
    
    using scoped_connector = final_act<disconnector>;

    auto make_scoped_connector(key_type key) const {
        return scoped_connector{disconnector{pimpl_, key}};
    }

    auto add_scoped(callback_type callback) {
        return make_scoped_connector(add(std::move(callback)));
    }
    
    void remove(key_type key) { pimpl_.remove(key); }
    void remove_all() { pimpl_->remove_all(); }

    template <typename... Us>
    void operator()(Us&&... us) const {
        pimpl_->operator()(std::forward<Us>(us)...);
    }

    bool empty() const { return pimpl_->empty(); }

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

        template <typename... Us>
        void operator()(Us&&... us) const {
            for (const auto& slot : slots_) {
                slot.second(std::forward<Us>(us)...);
            }
        }

        bool empty() const { return slots_.empty(); }

    private:
        key_type current_key_{0};
        std::unordered_map<key_type, callback_type> slots_;
    };

    /// We use a shared pointer here, so that scoped_connectors can make their
    /// own shared_ptrs, thereby increasing the lifespan of the impl object and
    /// ensuring that the disconnector will not attempt to dereference a
    /// previously-deleted pointer.
    std::shared_ptr<impl> pimpl_;
};

}  // namespace util
