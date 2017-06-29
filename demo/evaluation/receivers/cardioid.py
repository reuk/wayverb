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
    fig, (ax0, ax1) = plt.subplots(nrows=2, sharex=True, sharey=True)

    cmap = plt.get_cmap('viridis')

    def plt_file(ax, file_name, name):
        sndfile = pysndfile.PySndfile(file_name, 'r')
        if sndfile.channels() != 1:
            raise RuntimeError('please only load mono files')
        Fs = sndfile.samplerate()
        signal = sndfile.read_frames()

        time = np.arange(len(signal)) / float(Fs)
        ax.plot(time, signal)
        ax.text(0.001, 0.75, name)

    ax1.set_xlabel('time / s')
    ax1.set_xlim([0, 0.05])
    fig.subplots_adjust(hspace=0)
    plt.setp([a.get_xticklabels() for a in fig.axes[:-1]], visible=False)

    times = [
        3.0 / 340.0,
        5.0 / 340.0,
        11.0 / 340.0,
        13.0 / 340.0]

    for ax in fig.axes:
        ax.set_ylabel('amplitude')

        for t in times:
            ax.axvline(t, linestyle='dotted', color='red')

    plt_file(ax0, 'away.wav', 'away')
    plt_file(ax1, 'toward.wav', 'toward')

    plt.suptitle('Early Response for Cardoid Receivers Pointing Toward and Away from Source')

    #plt.tight_layout()
    #plt.subplots_adjust(top=0.9)

    plt.show()
    if render:
        plt.savefig('cardioid.svg', bbox_inches='tight', dpi=96, format='svg')

if __name__ == '__main__':
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'legend.fontsize': 12,
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    main()
