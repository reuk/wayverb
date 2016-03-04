import numpy as np
import matplotlib
render = False
if render:
    matplotlib.use('pgf')
import matplotlib.pyplot as plt
from string import split
import scipy.signal as sig
import wave
import math

def show_graph(free_field_file, subbed_file):
    files = [ free_field_file, subbed_file, ]

    def get_signals(f):
        spf = wave.open(f)
        signal = spf.readframes(-1)
        return np.fromstring(signal, "Int16")

    signals = [get_signals(i) for i in files]

    n = signals[0].size

    ffts = [np.fft.rfft(i) for i in signals]
    freq = np.fft.rfftfreq(n)

    def do_plot(a):
        plt.plot(np.resize(freq, n / 4), np.resize(np.abs(a), n / 4))

    for fft in ffts:
        do_plot(fft)

    plt.show()

    do_plot(ffts[1] / ffts[0])

    plt.show()
    if render:
        plt.savefig(this_file + ".plot.pdf", bbox_inches="tight")


def main():
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'font.monospace': ['Input Mono Condensed'],
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    files = [[
            "/Users/reuben/dev/waveguide/build/boundary_test/flat_windowed_free_field.wav",
            "/Users/reuben/dev/waveguide/build/boundary_test/flat_windowed_subbed.wav",
        ], [
            "/Users/reuben/dev/waveguide/build/boundary_test/filtered_windowed_free_field.wav",
            "/Users/reuben/dev/waveguide/build/boundary_test/filtered_windowed_subbed.wav",
        ]]

    for a, b in files:
        show_graph(a, b)

if __name__ == "__main__":
    main()
