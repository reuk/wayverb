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
    fig, ax = plt.subplots(nrows=2, sharex=False)

    cmap = plt.get_cmap('viridis')

    sndfile = pysndfile.PySndfile('vault.wav', 'r')
    if sndfile.channels() != 1:
        raise RuntimeError('please only load mono files')
    Fs = sndfile.samplerate()
    signal = sndfile.read_frames()
    NFFT=2048
    pxx, freq, time = mlab.specgram(signal, NFFT=NFFT, noverlap=NFFT/2, Fs=Fs)

    def do_impulse_plot():
        time = np.arange(len(signal)) / float(Fs)
        ax[0].plot(time, signal)

        ax[0].set_xlim([0, 0.1])

        ax[0].set_xlabel('time / s')
        ax[0].set_ylabel('amplitude')

        time = 7.12 / 340.0
        ax[0].axvline(time, linestyle='dotted', color='red')
        ax[0].text(time + 0.0005, -0.5, str.format('{0:.3g} s (earliest possible diffraction time)', time))

    def do_spec_plot():
        Z = 10 * np.log10(pxx)

        vmin = -200
        vmax = np.nanmax(Z)

        im = ax[1].pcolormesh(time, freq, Z, cmap=cmap, vmin=vmin, vmax=vmax, rasterized=True)
        ax[1].set_xlim([0, 1])

        ax[1].set_ylim(20, 20000)
        ax[1].set_yscale('log')

        ax[1].set_xlabel('time / s')
        ax[1].set_ylabel('frequency / Hz')

        cb = fig.colorbar(im)
        cb.set_label('dB')

    do_impulse_plot()
    do_spec_plot()

    plt.suptitle('Early Response in Vault Model')

    plt.tight_layout()
    plt.subplots_adjust(top=0.9)

    plt.show()
    if render:
        plt.savefig('vault_response.svg', bbox_inches='tight', dpi=96, format='svg')

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
