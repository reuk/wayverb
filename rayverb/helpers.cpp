#include "helpers.h"

#ifdef DIAGNOSTIC
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#endif

#include <iostream>
#include <fstream>
#include <cmath>
#include <random>
#include <chrono>

using namespace std;

#ifdef DIAGNOSTIC
using namespace rapidjson;

void print_diagnostic
(   unsigned long nrays
,   unsigned long nreflections
,   const vector <Impulse> & impulses
,   const string & fname
)
{
    ofstream out (fname);

    for (auto i = 0; i != nrays; ++i)
    {
        StringBuffer stringBuffer;
        Writer <StringBuffer> writer (stringBuffer);

        writer.StartArray();
        for (auto j = 0; j != nreflections; ++j)
        {
            Impulse reflection = impulses [i * nreflections + j];

            writer.StartObject();

            writer.String ("position");
            writer.StartArray();
            for (auto k = 0; k != 3; ++k)
                writer.Double (reflection.position.s [k]);
            writer.EndArray();

            writer.String ("volume");
            float average = 0;
            for (auto k = 0; k != 8; ++k)
                average += reflection.volume.s [k];
            average /= 8;
            writer.Double (average);

            writer.EndObject();
        }
        writer.EndArray();

        out << stringBuffer.GetString() << endl;
    }
}
#endif

// -1 <= z <= 1, -pi <= theta <= pi
cl_float3 spherePoint (float z, float theta)
{
    const float ztemp = sqrtf (1 - z * z);
    return (cl_float3) {{ztemp * cosf (theta), ztemp * sinf (theta), z, 0}};
}

vector <cl_float3> getRandomDirections (unsigned long num)
{
    vector <cl_float3> ret (num);
    uniform_real_distribution <float> zDist (-1, 1);
    uniform_real_distribution <float> thetaDist (-M_PI, M_PI);
    auto seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine engine (seed);

    for (auto && i : ret)
        i = spherePoint (zDist (engine), thetaDist (engine));

    return ret;
}
