#pragma once

#include <iosfwd>

class progress_bar final {
public:
    progress_bar(std::ostream& os, int expected);

    int get_expected() const;
    int get_count() const;

    int operator+=(int i);

private:
    /// from 0-1
    void draw(double progress);
    void draw_percentage(double progress);
    void draw_bar(double progress);

    std::ostream& os;
    int expected;
    int count{0};
};
