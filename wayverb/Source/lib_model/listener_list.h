#pragma once

#include <functional>
#include <unordered_set>

namespace std {
template <typename T> struct hash<std::reference_wrapper<T>> {
  using argument_type = typename std::reference_wrapper<T>;
  std::size_t operator()(argument_type const &s) noexcept {
    return reinterpret_cast<std::size_t>(&s.get());
  }
};

template <typename T>
bool operator==(const std::reference_wrapper<T> &a,
                const std::reference_wrapper<T> &b) {
  return &a.get() == &b.get();
}
}

namespace model {

template <typename Listener> class ListenerList {
public:
  ListenerList() = default;
  ListenerList(const ListenerList &) = delete;
  ListenerList &operator=(const ListenerList &) = delete;
  ListenerList(ListenerList &&) noexcept = delete;
  ListenerList &operator=(ListenerList &&) noexcept = delete;
  virtual ~ListenerList() noexcept = default;

  void add(Listener *listener) { listeners.insert(listener); }
  void remove(Listener *listener) { listeners.erase(listener); }

  auto size() const noexcept { return listeners.size(); }
  auto empty() const noexcept { return listeners.empty(); }

  void clear() { listeners.clear(); }

  bool contains(Listener *listener) const noexcept {
    return listeners.find(listener) != listeners.end();
  }

  template <typename Fn, typename... Ts> void call(Fn fn, Ts &&... ts) {
    for (auto &i : listeners) {
      (i->*fn)(std::forward<Ts>(ts)...);
    }
  }

private:
  std::unordered_set<Listener *> listeners;
};

} // namespace model
