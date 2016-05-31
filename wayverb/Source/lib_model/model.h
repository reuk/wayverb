#pragma once

#include "connector.h"

namespace model {

class ModelMember : public Broadcaster, public BroadcastListener {
public:
    ModelMember(ModelMember *owner)
            : owner(owner) {
    }

    ModelMember(const ModelMember &) = delete;
    ModelMember &operator=(const ModelMember &) = delete;
    ModelMember(ModelMember &&) noexcept = delete;
    ModelMember &operator=(ModelMember &&) noexcept = delete;
    virtual ~ModelMember() noexcept = default;

    void receive_broadcast(Broadcaster *b) override {
        broadcast();
    }

    ModelMember *get_owner() const {
        return owner;
    }

private:
    ModelMember *owner;
    BroadcastConnector owner_connector{this, owner};
};

template <typename T>
class ModelValue : public ModelMember {
public:
    using ModelMember::ModelMember;

    operator T() const {
        return get();
    }

    virtual void reseat(T &u) = 0;
    virtual T get() const = 0;
    virtual void set(const T &u,
                     bool do_notify = true) = 0;
};

}  // namesapce model
