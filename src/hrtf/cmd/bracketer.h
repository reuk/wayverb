#pragma once

#include <ostream>

class bracketer final {
public:
    bracketer(std::ostream& os, const char* enter, const char* exit)
            : os_{os}
            , exit_{exit} {
        os_ << enter;
    }

    ~bracketer() noexcept { os_ << exit_; }

private:
    std::ostream& os_;
    const char* exit_;
};

