import numpy as np
import matplotlib
render = False
if render:
    matplotlib.use('pgf')
import matplotlib.pyplot as plt
from string import split
import scipy.signal as signal
import wave
import math

from boundary_modelling import get_notch_coeffs, series_coeffs, db2a, a2db


def main():
    files = [
        ("raytracer", "/Users/reuben/dev/waveguide/tests/hybrid_test/output/raytracer_processed.wav"),
        ("waveguide", "/Users/reuben/dev/waveguide/tests/hybrid_test/output/waveguide_processed.wav"),
    ]

    sr = 44100

    def get_signals(f):
        spf = wave.open(f)
        signal = spf.readframes(-1)
        return np.fromstring(signal, "Int16")

    signals = [(tag, get_signals(i)) for tag, i in files]

    n = signals[0][1].size

    ffts = [(tag, np.fft.rfft(i)) for tag, i in signals]
    freq = np.fft.rfftfreq(n)

    plt.figure()

    num_bins_to_plot = 1000

    def do_plot(tag, a):
        plt.plot(
            np.resize(
                freq, num_bins_to_plot), np.resize(
                a, num_bins_to_plot), label=tag)

    for tag, fft in ffts:
        scaled = np.abs(fft) / n
        do_plot(tag, scaled)
    plt.legend()

    plt.show()


if __name__ == "__main__":
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'font.monospace': ['Input Mono Condensed'],
        'legend.fontsize': 10,
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    main()
