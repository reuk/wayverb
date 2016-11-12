#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

namespace model {

class Broadcaster;

class BroadcastListener {
public:
    BroadcastListener() = default;
    BroadcastListener(const BroadcastListener&) = default;
    BroadcastListener& operator=(const BroadcastListener&) = default;
    BroadcastListener(BroadcastListener&&) noexcept = default;
    BroadcastListener& operator=(BroadcastListener&&) noexcept = default;
    virtual ~BroadcastListener() noexcept = default;

    virtual void receive_broadcast(Broadcaster* broadcaster) = 0;
};

class Broadcaster {
public:
    using Listener = BroadcastListener;

    inline void broadcast() {
        listener_list.call(&Listener::receive_broadcast, this);
    }

    inline void addListener(Listener* l) {
        listener_list.add(l);
    }

    inline void removeListener(Listener* l) {
        listener_list.remove(l);
    }

private:
    ListenerList<BroadcastListener> listener_list;
};

}  // namespace model