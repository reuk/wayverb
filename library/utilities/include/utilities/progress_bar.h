#pragma once

#include <iostream>

class progress_bar final {
public:
    progress_bar(std::ostream& os = std::cerr);

    void set_progress(float progress);

private:
    std::ostream& os_;
};

void set_progress(progress_bar& pb, float progress);
void set_progress(progress_bar& pb, int step, int steps);
