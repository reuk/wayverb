#pragma once

#include "sndfile.hh"

#include <string>
#include <vector>

struct snd {
    static void write(const std::string& fname,
                      const std::vector<std::vector<float>>& signal,
                      double sample_rate,
                      size_t bit_depth);

private:
    static void write(const std::string& fname,
                      const std::vector<std::vector<float>>& outdata,
                      float sr,
                      int bd,
                      int ftype);

    static int get_file_format(const std::string& fname);
    static int get_file_depth(int bitDepth);
};
