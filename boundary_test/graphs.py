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

import sys
sys.path.append("python")

from boundary_modelling import get_notch_coeffs, series_coeffs
from filter_stability import to_filter_coefficients, Surface

COURANT = 1 / np.sqrt(3)
COURANT_SQ = COURANT * COURANT


def compute_boundary_coefficients(b_imp, a_imp, azimuth, elevation):
    # b_imp - b coefficients of the boundary impedance filter
    # a_imp - a coefficients of the boundary impedance filter
    # azimuth (theta)
    # elevation (phi)
    ca = np.cos(azimuth)
    ce = np.cos(elevation)

    num = [b * ca * ce - a for b, a in zip(b_imp, a_imp)]
    den = [b * ca * ce + a for b, a in zip(b_imp, a_imp)]

    a0 = den[0]

    num = [i / a0 for i in num]
    den = [i / a0 for i in den]

    return num, den


def frequency_plot(num, den, sr, label):
    w, h = signal.freqz(num, den)
#    w *= sr / 2 * np.pi
    n = len(w)
    w.resize(n / 2)
    h.resize(n / 2)
    w /= np.pi * 2

    plt.ylabel('Amplitude')
    plt.xlabel('Normalized Frequency')
#    plt.semilogx(w, 20 * np.log10(np.abs(h)), label=label)
#    plt.plot(w, 20 * np.log10(np.abs(h)), label=label)
    plt.plot(w, np.abs(h), label=label)
    plt.axvline(0.196)


def base_coeffs(i, sr):
    return series_coeffs(to_filter_coefficients(Surface(i, i), sr))


def surface_filter_plot(i, sr):
    num, den = base_coeffs(i, sr)
    frequency_plot(num, den, sr, "specified impedence filter curve")


def boundary_coefficient_plot(i, sr, azimuth, elevation):
    num, den = base_coeffs(i, sr)
    num, den = compute_boundary_coefficients(num, den, azimuth, elevation)
    frequency_plot(num, den, sr, "predicted")


def show_graph(free_field_file, subbed_file, surface, azimuth, elevation):
    sr = 44100

    files = [("free", free_field_file), ("reflected", subbed_file), ]

    def get_signals(f):
        spf = wave.open(f)
        signal = spf.readframes(-1)
        return np.fromstring(signal, "Int16")

    signals = [(tag, get_signals(i)) for tag, i in files]

    n = signals[0][1].size

    ffts = [(tag, np.fft.rfft(i)) for tag, i in signals]
    freq = np.fft.rfftfreq(n)

    plt.subplot(3, 1, 1)
    plt.title(subbed_file)

    def do_plot(arg):
        tag, a = arg
        plt.plot(np.resize(freq, n / 4), np.resize(np.abs(a), n / 4), label=tag)

    for fft in ffts:
        do_plot(fft)

    cutoff = 0.196

    plt.axvline(cutoff)
    plt.legend()

    plt.subplot(3, 1, 2)
    do_plot(("divided", ffts[1][1] / ffts[0][1]))
    boundary_coefficient_plot(surface, sr, azimuth, elevation)
    plt.axvline(cutoff)
    plt.legend()
    #plt.ylim([0, 2])

    plt.subplot(3, 1, 3)
    surface_filter_plot(surface, sr)
    plt.legend()

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

    az = 0
    el = 0
#    az = 0.7854
#    el = 0.5236
#    az = 1.047
#    el = 1.047

    root = "/Users/reuben/dev/waveguide/boundary_test/output/"
    azel = "_az_" + str(az) + "_el_" + str(el)

    suffix_free = "_windowed_free_field.wav"
    suffix_subb = "_windowed_subbed.wav"

    names = [
        ("flat",
            [1, 1, 1, 1, 1, 1, 1, 1]),
        ("anechoic",
            [0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001]),
        ("filtered_1",
            [1, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001]),
        ("filtered_2",
            [0.001, 1, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001]),
        ("filtered_3",
            [0.001, 0.001, 1, 0.001, 0.001, 0.001, 0.001, 0.001]),
        ("filtered_4",
            [0.4, 0.3, 0.5, 0.8, 0.9, 1, 1, 1]),
        ("filtered_5",
            [0.001, 1, 1, 1, 1, 1, 1, 1]),
        ("filtered_6",
            [1, 0.001, 1, 1, 1, 1, 1, 1]),
        ("filtered_7",
            [1, 1, 0.001, 1, 1, 1, 1, 1]),
    ]

    files = [(root + i + azel + suffix_free, root + i + azel + suffix_subb, j)
             for i, j in names]

    for a, b, c in files:
        show_graph(a, b, c, az, el)

if __name__ == "__main__":
    main()
