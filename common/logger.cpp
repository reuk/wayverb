#include "logger.h"

Bracketer::Bracketer(std::ostream& os)
        : os(os) {
    os << "[  ";
}
Bracketer::~Bracketer() noexcept {
    os << "]";
}

void Logger::restart() {
    std::ofstream of(fname, std::ofstream::trunc);
}

std::mutex Logger::mutex;
const std::string Logger::fname("logfile.txt");
