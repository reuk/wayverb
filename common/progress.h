#pragma once

#include <sstream>

class ProgressBar final {
public:
    ProgressBar(std::ostream& os, int expected);

    int get_expected() const;
    int get_count() const;

    int operator+=(int i);
    int operator++();

private:
    /// from 0-1
    void draw(double progress);
    void draw_percentage(double progress);
    void draw_bar(double progress);

    std::ostream& os;
    int expected;
    int count{0};
};
