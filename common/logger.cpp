#include "logger.h"

void Logger::restart() {
    std::ofstream of(fname, std::ofstream::trunc);
}

std::mutex Logger::mutex;
const std::string Logger::fname("logfile.txt");
