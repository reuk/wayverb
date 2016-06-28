#pragma once

#include "broadcaster.hpp"

namespace model {

template <typename Broadcaster>
struct ListenerFunctionTrait {
    using Listener = typename Broadcaster::Listener;

    static void add_listener(Broadcaster *const b, Listener *const l) {
        b->addListener(l);
    }

    static void remove_listener(Broadcaster *const b, Listener *const l) {
        b->removeListener(l);
    }
};

template <>
struct ListenerFunctionTrait<ChangeBroadcaster> {
    using Listener = ChangeListener;

    static void add_listener(ChangeBroadcaster *const b, Listener *const l) {
        b->addChangeListener(l);
    }

    static void remove_listener(ChangeBroadcaster *const b, Listener *const l) {
        b->removeChangeListener(l);
    }
};

template <typename Broadcaster,
          typename Listener =
                  typename ListenerFunctionTrait<Broadcaster>::Listener>
class Connector {
public:
    Connector(Broadcaster *const cb, Listener *const cl)
            : cb(cb)
            , cl(cl) {
        if (cb && cl) {
            ListenerFunctionTrait<Broadcaster>::add_listener(cb, cl);
        }
    }

    Connector(const Connector &) = delete;
    Connector &operator=(const Connector &) = delete;
    Connector(Connector &&) noexcept = delete;
    Connector &operator=(Connector &&) noexcept = delete;

    virtual ~Connector() noexcept {
        if (cb && cl) {
            ListenerFunctionTrait<Broadcaster>::remove_listener(cb, cl);
        }
    }

    Broadcaster *const get_broadcaster() const {
        return cb;
    }
    Listener *const get_listener() const {
        return cl;
    }

private:
    Broadcaster *const cb;
    Listener *const cl;
};

class BroadcastConnector : public Connector<Broadcaster> {
public:
    using Connector<Broadcaster>::Connector;
    void trigger() const {
        get_listener()->receive_broadcast(get_broadcaster());
    }
};

}  // namespace model
