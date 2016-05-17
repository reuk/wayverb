#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

namespace model {

template <typename Broadcaster>
struct ListenerFunctionTrait {
    template <typename Listener>
    static void add_listener(Broadcaster* const b, Listener* const l) {
        b->addListener(l);
    }

    template <typename Listener>
    static void remove_listener(Broadcaster* const b, Listener* const l) {
        b->removeListener(l);
    }
};

template <>
struct ListenerFunctionTrait<ChangeBroadcaster> {
    template <typename Listener>
    static void add_listener(ChangeBroadcaster* const b, Listener* const l) {
        b->addChangeListener(l);
    }

    template <typename Listener>
    static void remove_listener(ChangeBroadcaster* const b, Listener* const l) {
        b->removeChangeListener(l);
    }
};

template <typename Broadcaster,
          typename Listener = typename Broadcaster::Listener>
class Connector {
public:
    Connector(Broadcaster* const cb, Listener* const cl)
            : cb(cb)
            , cl(cl) {
        if (cb && cl) {
            ListenerFunctionTrait<Broadcaster>::add_listener(cb, cl);
        }
    }

    Connector(const Connector&) = delete;
    Connector& operator=(const Connector&) = delete;
    Connector(Connector&&) noexcept = delete;
    Connector& operator=(Connector&&) noexcept = delete;

    virtual ~Connector() noexcept {
        if (cb && cl) {
            ListenerFunctionTrait<Broadcaster>::remove_listener(cb, cl);
        }
    }

private:
    Broadcaster* const cb;
    Listener* const cl;
};

using ChangeConnector = Connector<ChangeBroadcaster, ChangeListener>;

class ModelMember : public ChangeListener, public ChangeBroadcaster {
public:
    ModelMember(ModelMember* owner)
            : owner(owner) {
    }

    ModelMember(const ModelMember&) = delete;
    ModelMember& operator=(const ModelMember&) = delete;
    ModelMember(ModelMember&&) noexcept = delete;
    ModelMember& operator=(ModelMember&&) noexcept = delete;
    virtual ~ModelMember() noexcept = default;

    void changeListenerCallback(ChangeBroadcaster* cb) override {
        notify();
    }
    void notify(bool do_notify = true) {
        if (do_notify) {
            sendSynchronousChangeMessage();
        }
    }

    ModelMember* get_owner() const {
        return owner;
    }

private:
    ModelMember* owner;
    ChangeConnector owner_connector{this, owner};
};

template <typename T>
class ModelValue : public ModelMember {
public:
    using ModelMember::ModelMember;

    virtual const T& get_value() const = 0;
    virtual void set_value(const T& u, bool do_notify) = 0;
};

}  // namespace model