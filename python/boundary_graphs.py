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


def main():
    sr = 44100
    nyquist = sr / 2

    max_frequency = sr * 0.196

    for i in [
        [0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001],
        [1, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001],
        [0.001, 1, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001],
        [0.001, 0.001, 1, 0.001, 0.001, 0.001, 0.001, 0.001],
        [0.4, 0.3, 0.5, 0.8, 0.9, 1, 1, 1],
        [1, 1, 1, 1, 1, 1, 1, 1],
    ]:
        num, den, = series_coeffs(to_filter_coefficients(Surface(i, i), sr))
        w, h = signal.freqz(num, den)
        w *= nyquist / np.pi

        plt.subplot(2, 2, 1)
        plt.ylabel('Amplitude Response (dB)')
        plt.xlabel('Frequency (Hz)')
        plt.semilogx(w, 20 * np.log10(np.abs(h)))

        plt.subplot(2, 2, 3)
        zplane(num, den)

        num, den = compute_boundary_coefficients(num, den, 0, 0)
        w, h = signal.freqz(num, den)
        w *= nyquist / np.pi

        plt.subplot(2, 2, 2)
        plt.ylabel('Amplitude Response (dB)')
        plt.xlabel('Frequency (Hz)')
        plt.semilogx(w, 20 * np.log10(np.abs(h)))

        plt.subplot(2, 2, 4)
        zplane(num, den)

        plt.show()


def show_graph(free_field_file, subbed_file):
    files = [("free", free_field_file), ("reflected", subbed_file), ]

    def get_signals(f):
        spf = wave.open(f)
        signal = spf.readframes(-1)
        return np.fromstring(signal, "Int16")

    signals = [(tag, get_signals(i)) for tag, i in files]

    n = signals[0][1].size

    ffts = [(tag, np.fft.rfft(i)) for tag, i in signals]
    freq = np.fft.rfftfreq(n)

    plt.subplot(2, 1, 1)
    plt.title(subbed_file)

    def do_plot(arg):
        tag, a = arg
        plt.plot(np.resize(freq, n / 4), np.resize(np.abs(a), n / 4), label=tag)

    for fft in ffts:
        do_plot(fft)

    plt.legend()

    plt.subplot(2, 1, 2)
    do_plot(("divided", ffts[1][1] / ffts[0][1]))

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

    root = "/Users/reuben/dev/waveguide/boundary_test/output/"
    azel = "_az_0_el_0"

    suffix_free = "_windowed_free_field.wav"
    suffix_subb = "_windowed_subbed.wav"

    names = [
        "flat",
        "anechoic",
        "filtered_1",
        "filtered_2",
        "filtered_3",
        "filtered_4"]

    files = [[root + i + azel + suffix_free, root + i + azel + suffix_subb]
             for i in names]

    for a, b in files:
        show_graph(a, b)

if __name__ == "__main__":
    main()
