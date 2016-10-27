#pragma once

#include <iostream>

class progress_bar final {
public:
    progress_bar(std::ostream& os = std::cerr);

    //  We print a newline on destruction so that separate progress bars get
    //  separated out.
    ~progress_bar() noexcept;

    void set_progress(float progress);

private:
    std::ostream& os_;
};

void set_progress(progress_bar& pb, float progress);
void set_progress(progress_bar& pb, int step, int steps);
