#pragma once

#define USE_JUCE_LISTENER_LIST

#ifdef USE_JUCE_LISTENER_LIST
#include "../JuceLibraryCode/JuceHeader.h"

namespace model {

class Broadcaster;

class BroadcastListener : public ChangeListener {
public:
    inline void changeListenerCallback(ChangeBroadcaster* cb) override;
    virtual void receive_broadcast(Broadcaster* broadcaster) = 0;
};

class Broadcaster : public ChangeBroadcaster {
public:
    using Listener = BroadcastListener;

    inline void broadcast() {
        sendChangeMessage();
    }

    void add(Listener* l) {
        addChangeListener(l);
    }

    void remove(Listener* l) {
        removeChangeListener(l);
    }
};

inline void BroadcastListener::changeListenerCallback(ChangeBroadcaster* cb) {
    //  TODO icky hack
    receive_broadcast(dynamic_cast<Broadcaster*>(cb));
}

}  // namespace model

#else
#include "listener_list.h"
namespace model {

class BroadcastListener;

class Broadcaster : public ListenerList<BroadcastListener> {
public:
    using Listener = BroadcastListener;
    inline void broadcast() {
        call(&BroadcastListener::receive_broadcast, this);
    }
};

class BroadcastListener {
public:
    BroadcastListener() = default;
    BroadcastListener(const BroadcastListener &) = default;
    BroadcastListener &operator=(const BroadcastListener &) = default;
    BroadcastListener(BroadcastListener &&) noexcept = default;
    BroadcastListener &operator=(BroadcastListener &&) noexcept = default;
    virtual ~BroadcastListener() noexcept = default;

    virtual void receive_broadcast(Broadcaster *broadcaster) = 0;
};

}  // namespace model
#endif
