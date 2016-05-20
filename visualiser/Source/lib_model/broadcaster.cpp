#include "model_wrapper/broadcaster.h"

namespace model {

void Broadcaster::broadcast() {
  call(&BroadcastListener::receive_broadcast, this);
}

} // namespace model
