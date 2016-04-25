#pragma once

struct DoNothingCallback {
    virtual void operator()() const {
    }
};

template <typename T>
struct CallbackInterfaceAdapter : public DoNothingCallback {
    explicit CallbackInterfaceAdapter(const T& t = T())
            : t(t) {
    }
    void operator()() const override {
        t();
    };

private:
    const T& t;
};

template <typename Callback>
CallbackInterfaceAdapter<Callback> make_callback(const Callback& c) {
    return CallbackInterfaceAdapter<Callback>(c);
}
