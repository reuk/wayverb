#pragma once

#include "listener_list.h"

namespace model {
class Broadcaster;

struct BroadcastListener {
  BroadcastListener() = default;
  BroadcastListener(const BroadcastListener &) = default;
  BroadcastListener &operator=(const BroadcastListener &) = default;
  BroadcastListener(BroadcastListener &&) noexcept = default;
  BroadcastListener &operator=(BroadcastListener &&) noexcept = default;
  virtual ~BroadcastListener() noexcept = default;

  virtual void receive_broadcast(Broadcaster *broadcaster) = 0;
};

class Broadcaster : public ListenerList<BroadcastListener> {
public:
  using Listener = BroadcastListener;
  void broadcast();
};
} // namespace model
