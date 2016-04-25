#pragma once

#include "sndfile.hh"

#include <string>
#include <vector>

void write_sndfile(const std::string& fname,
                   const std::vector<std::vector<float>>& outdata,
                   float sr,
                   int bd,
                   int ftype);

int get_file_format(const std::string& fname);
int get_file_depth(int bitDepth);
