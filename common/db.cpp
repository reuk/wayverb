#include "db.h"

#include <cmath>

double a2db(double a) {
    return 20 * log10(a);
}

double db2a(double db) {
    return pow(10, db / 20);
}
