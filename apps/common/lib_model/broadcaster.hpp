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

class Broadcaster : ListenerList<BroadcastListener> {
public:
    using Listener = BroadcastListener;

    inline void broadcast() {
        call(&Listener::receive_broadcast, this);
    }

    inline void addListener(Listener* l) {
        add(l);
    }

    inline void removeListener(Listener* l) {
        remove(l);
    }
};

}  // namespace model