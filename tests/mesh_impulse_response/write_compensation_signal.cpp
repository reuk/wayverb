#include "compressed_waveguide.h"

#include "common/write_audio_file.h"

void write_compensation_signal() {
    ComputeContext c;
    compressed_rectangular_waveguide_program program(c.context, c.device);
    compressed_rectangular_waveguide waveguide(program, 500);

    //  run the waveguide with an impulsive hard source
    auto output = waveguide.run_hard_source(std::vector<float>{0, 1});

    //  write the file
    snd::write("mesh_impulse_response.wav", {output}, 44100, 32);
}

int main() {
    write_compensation_signal();
}
