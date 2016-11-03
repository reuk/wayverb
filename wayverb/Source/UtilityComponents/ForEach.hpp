#pragma once

template <typename It, typename... Ts, typename Func>
void for_each(Func&& f, It first, It last, Ts&&... ts) {
    while (first != last) {
        f(*first++, (*ts++)...);
    }
}