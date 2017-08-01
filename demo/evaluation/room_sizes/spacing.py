#!/usr/local/bin/python

import numpy as np
import matplotlib
render = True
if render:
    matplotlib.use('pgf')
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
from string import split
import scipy.signal as signal
import pysndfile
import math
import os
import re
import json


def main():
    files = [
            ("near (1m)", "large.wav", [1]),
            ("far (11.8m)", "large_spaced.wav", [np.sqrt(2**2 + 10**2 + 6**2)]),
            ]

    fig, axes = plt.subplots(nrows=len(files), sharex=True)

    cmap = plt.get_cmap('viridis')

    for (label, fname, distances), ax in zip(files, axes):
        sndfile = pysndfile.PySndfile(fname, 'r')
        if sndfile.channels() != 1:
            raise RuntimeError('please only load mono files')
        Fs = sndfile.samplerate()
        signal = sndfile.read_frames()

        time = np.arange(len(signal)) / float(Fs)
        ax.plot(time, signal)

        ax.set_xlim([0, 0.05])

        ax.set_title(label)
        ax.set_xlabel('time / s')
        ax.set_ylabel('amplitude')

        for dist in distances:
            time = dist / 340.0
            ax.axvline(time, linestyle='dotted', color='red')
            ax.text(time + 0.0005, -0.3, str.format('{0:.3g} s', time))

    plt.suptitle('Differences in Direct Contribution Time for Different Source/Receiver Spacings')

    plt.tight_layout()
    plt.subplots_adjust(top=0.9)

    plt.show()
    if render:
        plt.savefig('spacing_signals.svg', bbox_inches='tight', dpi=96, format='svg')

if __name__ == '__main__':
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'legend.fontsize': 12,
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    main()
