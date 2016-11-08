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

    member(const member&) = default;
    member(member&&) noexcept = default;

    member& operator=(const member&) = default;
    member& operator=(member&&) noexcept = default;

    using on_change = util::event<>;
    typename on_change::connection connect_on_change(
            typename on_change::callback_type t) {
        return on_change_.connect(std::move(t));
    }

    void notify() { on_change_(); }

    void connect(const std::tuple<Sub&...>& sub) {
        connections_ = make_connections(sub);
    }

protected:
    ~member() noexcept = default;

private:
    auto make_connections(const std::tuple<Sub&...>& sub) {
        return util::map(
                [&](auto& param) {
                    return on_change::scoped_connection{
                            param.connect_on_change([&] { notify(); })};
                },
                sub);
    }

    on_change on_change_;
    std::tuple<typename util::event<Sub&>::scoped_connection...> connections_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
