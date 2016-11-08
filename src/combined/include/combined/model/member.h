#pragma once

#include "utilities/event.h"
#include "utilities/map.h"

namespace wayverb {
namespace combined {
namespace model {

template <typename Derived, typename... Sub>
class member {
public:
    member() = default;

    /// Copy operations should default-construct on_change_, thereby ensuring
    /// the copy has no listeners.

    member(const member& other) {}
    member(member&&) noexcept = default;

    member& operator=(const member& other) = default;
    member& operator=(member&&) noexcept = default;

    using on_change = util::event<Derived&>;
    using connection = typename on_change::connection;
    using scoped_connection = typename on_change::scoped_connection;
    using callback_type = typename on_change::callback_type;

    connection connect_on_change(callback_type t) {
        return on_change_.connect(std::move(t));
    }

    void notify() { on_change_(*static_cast<Derived*>(this)); }

protected:
    ~member() noexcept = default;

    void connect(Sub&... sub) {
        const auto connect = [&](auto& param) {
            using scoped_connection =
                    typename util::event<decltype(param)>::scoped_connection;
            return scoped_connection{
                    param.connect_on_change([&](auto&) { notify(); })};
        };

        connections_ = std::make_tuple(connect(sub)...);
    }

private:
    on_change on_change_;
    std::tuple<typename util::event<Sub&>::scoped_connection...> connections_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
