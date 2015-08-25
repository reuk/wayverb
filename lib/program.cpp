#include "program.h"

using namespace std;

WaveguideProgram::WaveguideProgram(const cl::Context & context,
                                   bool build_immediate)
        : Program(context, source, build_immediate) {
}

const string WaveguideProgram::source{
#ifdef DIAGNOSTIC
    "#define DIAGNOSTIC\n"
#endif
    R"(
    #define NULL (0)

    typedef float2 Node;

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
    (   global Node * mesh
    ,   float r
    )
    {
        int3 pos = int3(get_global_id(0),
                        get_global_id(1),
                        get_global_id(2));
        int3 dim = int3(get_global_size(0),
                        get_global_size(1),
                        get_global_size(2));

        float temp = 0;

        switch (boundary(pos, dim)) {
            case INSIDE:
                temp = (mesh[get_index(int3(pos.x + 1, pos.y, pos.z), dim)].y +
                        mesh[get_index(int3(pos.x - 1, pos.y, pos.z), dim)].y +
                        mesh[get_index(int3(pos.x, pos.y + 1, pos.z), dim)].y +
                        mesh[get_index(int3(pos.x, pos.y - 1, pos.z), dim)].y +
                        mesh[get_index(int3(pos.x, pos.y, pos.z + 1), dim)].y +
                        mesh[get_index(int3(pos.x, pos.y, pos.z - 1), dim)].y) / 3;
                break;

            case MIN_X:
                temp = (1 + r) * mesh[get_index(int3(1, pos.y, pos.z), dim)].y;
                break;
            case MAX_X:
                temp = (1 + r) * mesh[get_index(int3(dim.x - 2, pos.y, pos.z), dim)].y;
                break;
            case MIN_Y:
                temp = (1 + r) * mesh[get_index(int3(pos.x, 1, pos.z), dim)].y;
                break;
            case MAX_Y:
                temp = (1 + r) * mesh[get_index(int3(pos.x, dim.y - 2, pos.z), dim)].y;
                break;
            case MIN_Z:
                temp = (1 + r) * mesh[get_index(int3(pos.x, pos.y, 1), dim)].y;
                break;
            case MAX_Z:
                temp = (1 + r) * mesh[get_index(int3(pos.x, pos.y, dim.z - 2), dim)].y;
                break;

            case CORNER:
            default:
                break;
        }

        size_t index = get_index(pos, dim);
        temp -= mesh[index].x;

        //  TODO
        temp = index;

        //  wait for all members to be done with reading from memory
        barrier(CLK_GLOBAL_MEM_FENCE);

        //  update memory
        global Node * node = mesh + index;
        node->x = node->y;
        node->y = temp;
    }
    )"};
