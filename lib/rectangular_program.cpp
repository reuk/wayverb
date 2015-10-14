#include "rectangular_program.h"

using namespace std;

RectangularProgram::RectangularProgram(const cl::Context & context,
                                       bool build_immediate)
        : Program(context, source, build_immediate) {
}

const string RectangularProgram::source{
#ifdef DIAGNOSTIC
    "#define DIAGNOSTIC\n"
#endif
    R"(
    typedef enum {
        MIN_X, MAX_X, MIN_Y, MAX_Y, MIN_Z, MAX_Z, CORNER, INSIDE,
    } Boundary;

    Boundary boundary(int3 pos, int3 dim) {
        bool min_x = pos.x == 0;
        bool min_y = pos.y == 0;
        bool min_z = pos.z == 0;
        bool max_x = pos.x == dim.x - 1;
        bool max_y = pos.y == dim.y - 1;
        bool max_z = pos.z == dim.z - 1;

        bool ux = min_x || max_x;
        bool uy = min_y || max_y;
        bool uz = min_z || max_z;

        if (!ux && !uy && !uz) {
            return INSIDE;
        }

        if (ux && !(uy || uz)) {
            return min_x ? MIN_X : MAX_X;
        }

        if (uy && !(ux || uz)) {
            return min_y ? MIN_Y : MAX_Y;
        }

        if (uz && !(ux || uy)) {
            return min_z ? MIN_Z : MAX_Z;
        }

        return CORNER;
    }

    size_t get_index(int3 pos, int3 dim) {
        return pos.x + pos.y * dim.x + pos.z * dim.x * dim.y;
    }

    kernel void waveguide
    (   global float * current
    ,   global float * previous
    ,   float r
    ,   unsigned long read
    ,   global float * output
    ) {
        int3 pos = (int3)(get_global_id(0),
                          get_global_id(1),
                          get_global_id(2));
        int3 dim = (int3)(get_global_size(0),
                          get_global_size(1),
                          get_global_size(2));

        size_t index = get_index(pos, dim);

        float temp = 0;

        switch (boundary(pos, dim)) {
            case INSIDE:
                temp += (current[get_index(pos + (int3)(-1, 0, 0), dim)] +
                         current[get_index(pos + (int3)(+1, 0, 0), dim)] +
                         current[get_index(pos + (int3)(0, -1, 0), dim)] +
                         current[get_index(pos + (int3)(0, +1, 0), dim)] +
                         current[get_index(pos + (int3)(0, 0, -1), dim)] +
                         current[get_index(pos + (int3)(0, 0, +1), dim)]) / 3;
                break;

            case MIN_X:
                temp += (1 + r) * current[get_index((int3)(1, pos.y, pos.z), dim)];
                break;
            case MAX_X:
                temp += (1 + r) * current[get_index((int3)(dim.x - 2, pos.y, pos.z), dim)];
                break;
            case MIN_Y:
                temp += (1 + r) * current[get_index((int3)(pos.x, 1, pos.z), dim)];
                break;
            case MAX_Y:
                temp += (1 + r) * current[get_index((int3)(pos.x, dim.y - 2, pos.z), dim)];
                break;
            case MIN_Z:
                temp += (1 + r) * current[get_index((int3)(pos.x, pos.y, 1), dim)];
                break;
            case MAX_Z:
                temp += (1 + r) * current[get_index((int3)(pos.x, pos.y, dim.z - 2), dim)];
                break;

            case CORNER:
            default:
                break;
        }

        temp -= previous[index];

        previous[index] = temp;

        if (index == read) {
            *output = previous[index];
        }
    }
    )"};
