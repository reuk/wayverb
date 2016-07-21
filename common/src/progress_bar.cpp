#include "common/progress_bar.h"

#include <ostream>

progress_bar::progress_bar(std::ostream& os, int expected)
        : os(os)
        , expected(expected) {
    draw(0);
}

void progress_bar::draw_percentage(double progress) {
    os << int(progress * 100) << "%";
}

void progress_bar::draw_bar(double progress) {
    constexpr auto width = 40;
    int filled = progress * width;
    int blank = width - filled;
    os << '[';
    for (auto i = 0; i != filled; ++i) {
        os << '/';
    }
    for (auto i = 0; i != blank; ++i) {
        os << ' ';
    }
    os << "] ";
}

void progress_bar::draw(double progress) {
    progress = std::min(1.0, std::max(0.0, progress));

    os << '\r';
    draw_bar(progress);
    draw_percentage(progress);
    os << std::flush;
}

int progress_bar::operator+=(int i) {
    count += i;
    draw(count / (expected - 1.0));
    if (count >= expected) {
        os << std::endl;
    }
    return count;
}
