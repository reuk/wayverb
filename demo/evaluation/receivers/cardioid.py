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

    fig, axes = plt.subplots(nrows=1, sharex=True)

    cmap = plt.get_cmap('viridis')

    sndfile = pysndfile.PySndfile('cardioid_away.wav', 'r')
    if sndfile.channels() != 1:
        raise RuntimeError('please only load mono files')
    Fs = sndfile.samplerate()
    signal = sndfile.read_frames()

    time = np.arange(len(signal)) / float(Fs)
    axes.plot(time, signal)

    axes.set_xlim([0, 0.05])

    axes.set_xlabel('time / s')
    axes.set_ylabel('amplitude')

    time = 3.0 / 340.0
    axes.axvline(time, linestyle='dotted', color='red')
    axes.text(time + 0.0005, -0.2, str.format('{0:.3g} s', time))

    time = 5.0 / 340.0
    axes.axvline(time, linestyle='dotted', color='red')
    axes.text(time + 0.0005, -0.3, str.format('{0:.3g} s', time))

    time = 11.0 / 340.0
    axes.axvline(time, linestyle='dotted', color='red')
    axes.text(time + 0.0005, -0.2, str.format('{0:.3g} s', time))

    time = 13.0 / 340.0
    axes.axvline(time, linestyle='dotted', color='red')
    axes.text(time + 0.0005, -0.3, str.format('{0:.3g} s', time))

    plt.suptitle('Early Response for Cardoid Receiver Pointing Away From Source')

    plt.tight_layout()
    plt.subplots_adjust(top=0.9)

    plt.show()
    if render:
        plt.savefig('cardioid.svg', bbox_inches='tight', dpi=96, format='svg')

if __name__ == '__main__':
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'font.monospace': ['Input Mono Condensed'],
        'legend.fontsize': 12,
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    main()
