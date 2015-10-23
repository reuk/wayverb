#pragma once

#include "sndfile.hh"

#include <vector>
#include <string>

void write_sndfile(const std::string & fname,
                   const std::vector<std::vector<float>> & outdata,
                   float sr,
                   unsigned long bd,
                   unsigned long ftype);

unsigned long get_file_format(const std::string & fname);
unsigned long get_file_depth(unsigned long bitDepth);
