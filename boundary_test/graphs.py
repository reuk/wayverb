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

def main():
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'font.monospace': ['Input Mono Condensed'],
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    files = [
            "/Users/reuben/dev/waveguide/build/boundary_test/windowed_free_field.wav",
            "/Users/reuben/dev/waveguide/build/boundary_test/windowed_subbed.wav",
            ]

    def get_signals(f):
        spf = wave.open(f)
        signal = spf.readframes(-1)
        return np.fromstring(signal, "Int16")

    signals = [get_signals(i) for i in files]

    n = signals[0].size

    ffts = [np.fft.rfft(i) for i in signals]
    freq = np.fft.rfftfreq(n)

#    for fft in ffts:
#        plt.plot(freq, np.abs(fft))

    div = ffts[1] / ffts[0]
    plt.plot(freq, np.abs(div))

#    quot, rem = sig.deconvolve(signals[0], signals[1])
#    plt.plot(freq, np.abs(np.fft.rfft(rem)))

    plt.show()
    if render:
        plt.savefig(this_file + ".plot.pdf", bbox_inches="tight")

if __name__ == "__main__":
    main()
