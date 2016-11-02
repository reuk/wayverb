#include "utilities/progress_bar.h"

#include <ostream>

namespace util {

namespace {
void draw_percentage(std::ostream& os, float progress) {
    os << static_cast<size_t>(progress * 100) << "%";
}

void draw_bar(std::ostream& os, float progress) {
    constexpr auto width = 53ul;
    const size_t filled = progress * width;
    const auto blank = width - filled;
    os << '[';
    for (auto i = 0ul; i != filled; ++i) {
        os << '/';
    }
    for (auto i = 0ul; i != blank; ++i) {
        os << ' ';
    }
    os << "] ";
}
}  // namespace

progress_bar::progress_bar(std::ostream& os)
        : os_{os} {}

progress_bar::~progress_bar() noexcept { os_ << '\n'; }

void progress_bar::set_progress(float progress) {
    progress = std::max(0.0f, std::min(1.0f, progress));

    os_ << '\r';
    draw_bar(os_, progress);
    draw_percentage(os_, progress);
    os_ << std::flush;
}

void set_progress(progress_bar& pb, float progress) {
    pb.set_progress(progress);
}

void set_progress(progress_bar& pb, int step, int steps) {
    set_progress(pb, step / (steps - 1.0));
}

}  // namespace util
