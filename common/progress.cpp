#include "progress.h"

ProgressBar::ProgressBar(std::ostream& os, int expected)
        : os(os)
        , expected(expected) {
    draw(0);
}

void ProgressBar::draw_percentage(double progress) {
    os << int(progress * 100) << "%";
}

void ProgressBar::draw_bar(double progress) {
    constexpr auto width = 40;
    int filled = progress * width;
    int blank = width - filled;
    os << '[';
    for (auto i = 0; i != filled; ++i)
        os << '/';
    for (auto i = 0; i != blank; ++i)
        os << ' ';
    os << "] ";
}

void ProgressBar::draw(double progress) {
    progress = std::min(1.0, std::max(0.0, progress));

    os << '\r';
    draw_bar(progress);
    draw_percentage(progress);
    os << std::flush;
}

int ProgressBar::operator+=(int i) {
    count += i;
    draw(count / (expected - 1.0));
    if (count >= expected)
        os << std::endl;
    return count;
}
int ProgressBar::operator++() {
    return operator+=(1);
}
