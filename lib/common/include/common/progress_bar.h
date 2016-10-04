#pragma once

#include <iosfwd>

class progress_bar final {
public:
    progress_bar(std::ostream& os, size_t expected);

    size_t get_expected() const;
    size_t get_count() const;

    size_t operator+=(size_t i);

private:
    /// from 0-1
    void draw(double progress);
    void draw_percentage(double progress);
    void draw_bar(double progress);

    std::ostream& os;
    size_t expected;
    size_t count{0};
};