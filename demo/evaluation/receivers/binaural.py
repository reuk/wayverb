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
            ("left ear", "left_ear.wav"),
            ("right ear", "right_ear.wav"),
            ]

    fig = plt.figure()

    for label, fname in files:
        sndfile = pysndfile.PySndfile(fname, 'r')
        if sndfile.channels() != 1:
            raise RuntimeError('please only load mono files')
        Fs = sndfile.samplerate()
        signal = sndfile.read_frames()

        time = np.arange(len(signal)) / float(Fs)
        plt.plot(time, signal, label=label)

    plt.legend(loc='lower center', ncol=2)

    plt.xlim([0.04, 0.1])

    plt.xlabel('time / s')
    plt.ylabel('amplitude')

    plt.suptitle('Comparison of Left and Right Ear Responses')

    #plt.tight_layout()

    plt.show()
    if render:
        plt.savefig('binaural_signals.svg', bbox_inches='tight', dpi=96, format='svg')

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
