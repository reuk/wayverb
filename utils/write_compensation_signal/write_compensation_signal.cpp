#include "compressed_waveguide.h"

#include "common/write_audio_file.h"

/// This is a short program to find the impulse response of a rectangular
/// waveguide mesh with the maximum permissable courant number.
/// This signal will be written to a file called 'mesh_impulse_response.wav'.

/// There is a tool in the `data` folder to write the signal into a .cpp source
/// so that it can be baked into the waveguide library.
/// But, because this might take a long time to run, I've decided to just
/// include a copy of the .wav in the repository.

/// This tool is included mainly so that you can see how the mesh ir is
/// generated, and to give you the option of generating a longer ir if you
/// decide that is necessary.

void write_compensation_signal(const std::string& fname, size_t n) {
    //  get a cl context and device
    compute_context cc;

    //  Creates a compressed waveguide with the specified centre-to-edge
    //  distance. For a distance n, this can be used to generate a mesh ir with
    //  length 2n.
    compressed_rectangular_waveguide waveguide(
            cc.get_context(), cc.get_device(), n);

    //  run the waveguide with an impulsive hard source
    auto output = waveguide.run_hard_source(aligned::vector<float>{0, 1});

    //  write the file
    snd::write(fname, {output}, 44100, 32);
}

int main() {
    write_compensation_signal("mesh_impulse_response.wav", 500);
}
