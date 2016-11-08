#pragma once

#include <functional>
#include <unordered_map>

namespace util {

template <typename... Ts>
class event final {
    class impl;
    using key_type = size_t;

public:
    event()
            : pimpl_{std::make_shared<impl>()} {}

    event(const event&) = delete;
    event(event&&) noexcept = default;

    event& operator=(const event&) = delete;
    event& operator=(event&&) noexcept = default;

    ~event() noexcept = default;

    using callback_type = std::function<void(Ts...)>;

    class connection final {
    public:
        connection() = default;
        connection(std::shared_ptr<impl> pimpl, callback_type callback)
                : pimpl_{std::move(pimpl)}
                , key_{pimpl_->connect(std::move(callback))} {}

        void disconnect() {
            pimpl_->disconnect(key_);
            pimpl_ = nullptr;
        }

        void block() { pimpl_->block(key_); }
        void unblock() { pimpl_->block(key_); }
        bool blocked() const { return pimpl_->blocked(key_); }

        explicit operator bool() const { return pimpl_ != nullptr; }

    private:
        std::shared_ptr<impl> pimpl_;
        key_type key_;
    };

    auto connect(callback_type callback) {
        return connection{pimpl_, std::move(callback)};
    }

    class scoped_connection final {
    public:
        scoped_connection() = default;
        explicit scoped_connection(connection connection)
                : connection_{std::move(connection)} {}

        void swap(scoped_connection& other) noexcept {
            using std::swap;
            swap(connection_, other.connection_);
        }

        scoped_connection(const scoped_connection&) = delete;
        scoped_connection(scoped_connection&& other) noexcept { swap(other); }

        scoped_connection& operator=(const scoped_connection&) = delete;
        scoped_connection& operator=(scoped_connection&& other) noexcept {
            swap(other);
            return *this;
        }

        ~scoped_connection() noexcept {
            if (connection_) {
                connection_.disconnect();
            }
        }

        void block() { connection_.block(); }
        void unblock() { connection_.unblock(); }
        bool blocked() const { return connection_.blocked(); }

        operator bool() const { return connection_; }

    private:
        connection connection_;
    };

    template <typename... Us>
    void operator()(Us&&... us) const {
        pimpl_->operator()(std::forward<Us>(us)...);
    }

    bool empty() const { return pimpl_->empty(); }

private:
    class impl final {
        struct callback_info final {
            callback_type callback;
            bool blocked{false};
        };

    public:
        impl() = default;

        impl(const impl&) = delete;
        impl(impl&&) noexcept = delete;

        impl& operator=(const impl&) = delete;
        impl& operator=(impl&&) noexcept = delete;

        auto connect(callback_type callback) {
            slots_.insert(std::make_pair(++current_key_,
                                         callback_info{std::move(callback)}));
            return current_key_;
        }

        void disconnect(key_type key) { slots_.erase(key); }

        void block(key_type key) {
            auto slot = slots_.find(key);
            if (slot != end(slots_)) {
                slot->second.blocked = true;
            }
        }

        void unblock(key_type key) {
            auto slot = slots_.find(key);
            if (slot != end(slots_)) {
                slot->second.blocked = false;
            }
        }

        bool blocked(key_type key) const {
            auto slot = slots_.find(key);
            if (slot != end(slots_)) {
                return slot->second.blocked;
            }
            throw std::logic_error{"no such key"};
        }

        template <typename... Us>
        void operator()(Us&&... us) const {
            for (const auto& slot : slots_) {
                if (!slot.second.blocked) {
                    slot.second.callback(std::forward<Us>(us)...);
                }
            }
        }

        bool empty() const { return slots_.empty(); }

    private:
        key_type current_key_{0};
        std::unordered_map<key_type, callback_info> slots_;
    };

    /// We use a shared pointer here, so that scoped_connectors can make their
    /// own shared_ptrs, thereby increasing the lifespan of the impl object and
    /// ensuring that the disconnector will not attempt to dereference a
    /// previously-deleted pointer.
    std::shared_ptr<impl> pimpl_;
};

}  // namespace util
