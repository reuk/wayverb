import numpy as np
import matplotlib
render = True
if render:
    matplotlib.use('pgf')
import matplotlib.pyplot as plt
from string import split
import scipy.signal as signal
import wave
import math

import sys
sys.path.append("python")

from boundary_modelling import get_notch_coeffs, series_coeffs, db2a, a2db
from filter_stability import to_filter_coefficients, Surface

COURANT = 1 / np.sqrt(3)
COURANT_SQ = COURANT * COURANT

USE_DB_AXES = True

CUTOFF = 0.196


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

    plt.ylabel('Amplitude / dB')
    plt.xlabel('Normalized Frequency')

    if USE_DB_AXES:
        plt.plot(w, a2db(np.abs(h)), label=label)
    else:
        plt.plot(w, np.abs(h), label=label)
    plt.axvline(CUTOFF)


def base_coeffs(i, sr):
    return series_coeffs(to_filter_coefficients(Surface(i, i), sr))


def impedance_coeffs(n, d, sr):
    num = [a + b for b, a in zip(n, d)]
    den = [a - b for b, a in zip(n, d)]

    a0 = den[0]

    num = [i / a0 for i in num]
    den = [i / a0 for i in den]

    return num, den


def surface_filter_plot(i, sr):
    num, den = base_coeffs(i, sr)
    frequency_plot(num, den, sr, "reflectance filter response")


def boundary_coefficient_plot(i, sr, azimuth, elevation):
    num, den = base_coeffs(i, sr)
    num, den = impedance_coeffs(num, den, sr)
    num, den = compute_boundary_coefficients(num, den, azimuth, elevation)
    frequency_plot(num, den, sr, "predicted response")


def show_graph(free_field_file, subbed_file, name, surface, azimuth, elevation):
    filter_frequency = 2000
    sr = filter_frequency * 4

    files = [("free", free_field_file), ("reflection response", subbed_file), ]

    def get_signals(f):
        spf = wave.open(f)
        signal = spf.readframes(-1)
        return np.fromstring(signal, "Int16")

    signals = [(tag, get_signals(i)) for tag, i in files]

    n = signals[0][1].size

    ffts = [(tag, np.fft.rfft(i)) for tag, i in signals]
    freq = np.fft.rfftfreq(n)

    def do_plot(tag, a):
        plt.plot(np.resize(freq, n / 4), np.resize(a, n / 4), label=tag)

    div = np.abs(ffts[1][1] / ffts[0][1])
    if USE_DB_AXES:
        div = a2db(div)
    do_plot("divided", div)
    boundary_coefficient_plot(surface, sr, azimuth, elevation)

    plt.title(
        " azimuth: " +
        str(azimuth) +
        ", elevation: " +
        str(elevation))
    surface_filter_plot(surface, sr)
    plt.legend()


def main():
    azel = [
        (0, 0),
        (0.7854, 0),
        (0.7854, 0.7854),
        (1.047, 1.047),
    ]

    root = "/Users/reuben/dev/waveguide/boundary_test/output/"
    suffix_free = "_windowed_free_field.wav"
    suffix_subb = "_windowed_subbed.wav"

    lo = 0.01
    hi = 0.9

    names = [
        ("anechoic",
            [lo, lo, lo, lo, lo, lo, lo, lo]),
        ("filtered_1",
            [hi, lo, lo, lo, lo, lo, lo, lo]),
        ("filtered_2",
            [lo, hi, lo, lo, lo, lo, lo, lo]),
        ("filtered_3",
            [lo, lo, hi, lo, lo, lo, lo, lo]),
        ("filtered_4",
            [0.4, 0.3, 0.5, 0.8, 0.9, hi, hi, hi]),
        ("filtered_5",
            [lo, hi, hi, hi, hi, hi, hi, hi]),
        ("filtered_6",
            [hi, lo, hi, hi, hi, hi, hi, hi]),
        ("filtered_7",
            [hi, hi, lo, hi, hi, hi, hi, hi]),
        ("flat",
            [hi, hi, hi, hi, hi, hi, hi, hi]),
    ]

    for az, el in azel:
        az_el = "az_" + str(az) + "_el_" + str(el)

        def make_fname(i, suffix):
            return root + az_el + "/" + i + "_" + az_el + suffix

        files = [(make_fname(i, suffix_free), make_fname(i, suffix_subb), i, j)
                 for i, j in names]

        for a, b, c, d in files:
            plt.figure()
            show_graph(a, b, c, d, az, el)
            plt.show()
            if render:
                plt.savefig(subbed_file + ".plot.pdf", bbox_inches="tight")


def one_page():
    azel = [
        (0, 0),
        (0.7854, 0),
        (0.7854, 0.7854),
        (1.047, 1.047),
    ]

    root = "/Users/reuben/dev/waveguide/tests/boundary_test/output/"
    suffix_free = "_windowed_free_field.wav"
    suffix_subb = "_windowed_subbed.wav"

    lo = 0.01
    hi = 0.9

    names = [
        ("anechoic",
            [lo, lo, lo, lo, lo, lo, lo, lo]),
        ("filtered_1",
            [hi, lo, lo, lo, lo, lo, lo, lo]),
        ("filtered_2",
            [lo, hi, lo, lo, lo, lo, lo, lo]),
        ("filtered_3",
            [lo, lo, hi, lo, lo, lo, lo, lo]),
        ("filtered_5",
            [lo, hi, hi, hi, hi, hi, hi, hi]),
        ("filtered_6",
            [hi, lo, hi, hi, hi, hi, hi, hi]),
        ("filtered_7",
            [hi, hi, lo, hi, hi, hi, hi, hi]),
        ("flat",
            [hi, hi, hi, hi, hi, hi, hi, hi]),
    ]

    for az, el in azel:
        az_el = "az_" + str(az) + "_el_" + str(el)

        def make_fname(i, suffix):
            return root + az_el + "/" + i + "_" + az_el + suffix

        files = [(make_fname(i, suffix_free), make_fname(i, suffix_subb), i, j)
                 for i, j in names]

        plt.figure(figsize=(8.27, 11.69))
        for (a, b, c, d), i in zip(files, range(len(files))):
            plt.subplot(4, 2, i + 1)
            show_graph(a, b, c, d, az, el)
        plt.tight_layout()
        plt.show()
        if render:
            plt.savefig(
                root +
                az_el +
                "/" +
                az_el +
                ".plot.pdf",
                bbox_inches="tight", dpi=300)


if __name__ == "__main__":
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'font.monospace': ['Input Mono Condensed'],
        'legend.fontsize': 7,
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    one_page()
