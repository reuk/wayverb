#include "logger.h"

using namespace std;
using namespace std::chrono;

void Logger::restart() {
    ofstream of(fname, ofstream::trunc);
}

mutex Logger::mutex;
const string Logger::fname("logfile.txt");
