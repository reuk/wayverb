#pragma once

#include <functional>
#include <mutex>
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
        connection(const std::shared_ptr<impl>& pimpl, callback_type callback)
                : pimpl_{pimpl}
                , key_{pimpl->connect(std::move(callback))} {}

        void swap(connection& other) noexcept {
            using std::swap;
            swap(pimpl_, other.pimpl_);
            swap(key_, other.key_);
        }

        connection(const connection&) = delete;
        connection(connection&& other) noexcept { swap(other); }

        connection& operator=(const connection&) = delete;
        connection& operator=(connection&& other) noexcept {
            swap(other);
            return *this;
        }

        void disconnect() {
            if (auto p = pimpl_.lock()) {
                p->disconnect(key_);
                pimpl_.reset();
            }
        }

        void block() {
            if (auto p = pimpl_.lock()) {
                p->block(key_);
            }
        }

        void unblock() {
            if (auto p = pimpl_.lock()) {
                p->block(key_);
            }
        }

        explicit operator bool() const { return !pimpl_.expired(); }

    private:
        std::weak_ptr<impl> pimpl_;
        key_type key_;
    };

    class scoped_connection final {
    public:
        scoped_connection() = default;
        explicit scoped_connection(connection connection)
                : connection{std::move(connection)} {}

        scoped_connection(const scoped_connection&) = delete;
        scoped_connection(scoped_connection&& other) noexcept = default;

        scoped_connection& operator=(const scoped_connection&) = delete;
        scoped_connection& operator=(scoped_connection&& other) noexcept =
                default;

        ~scoped_connection() noexcept { connection.disconnect(); }

        connection connection;
    };

    auto connect(callback_type callback) {
        return connection{pimpl_, std::move(callback)};
    }

    template <typename... Us>
    void operator()(Us&&... us) const {
        pimpl_->operator()(std::forward<Us>(us)...);
    }

    bool empty() const { return pimpl_->empty(); }
    size_t size() const { return pimpl_->size(); }

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
            std::lock_guard<std::mutex> lck{mutex_};
            slots_.insert(std::make_pair(++current_key_,
                                         callback_info{std::move(callback)}));
            return current_key_;
        }

        void disconnect(key_type key) {
            std::lock_guard<std::mutex> lck{mutex_};
            slots_.erase(key);
        }

        void block(key_type key) {
            std::lock_guard<std::mutex> lck{mutex_};
            auto slot = slots_.find(key);
            if (slot != end(slots_)) {
                slot->second.blocked = true;
            }
        }

        void unblock(key_type key) {
            std::lock_guard<std::mutex> lck{mutex_};
            auto slot = slots_.find(key);
            if (slot != end(slots_)) {
                slot->second.blocked = false;
            }
        }

        bool blocked(key_type key) const {
            std::lock_guard<std::mutex> lck{mutex_};
            auto slot = slots_.find(key);
            if (slot != end(slots_)) {
                return slot->second.blocked;
            }
            throw std::logic_error{"No such key."};
        }

        template <typename... Us>
        void operator()(Us&&... us) const {
            std::unordered_map<key_type, callback_info> copy;

            {
                std::lock_guard<std::mutex> lck{mutex_};
                copy = slots_;
            }

            for (const auto& slot : copy) {
                if (!slot.second.blocked) {
                    slot.second.callback(std::forward<Us>(us)...);
                }
            }
        }

        bool empty() const {
            std::lock_guard<std::mutex> lck{mutex_};
            return slots_.empty();
        }

        size_t size() const {
            std::lock_guard<std::mutex> lck{mutex_};
            return slots_.size();
        }

    private:
        mutable std::mutex mutex_;

        key_type current_key_{0};
        std::unordered_map<key_type, callback_info> slots_;
    };

    std::shared_ptr<impl> pimpl_;
};

}  // namespace util
