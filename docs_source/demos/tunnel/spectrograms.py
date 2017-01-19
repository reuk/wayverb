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

def get_specgram_data(fname):
    sndfile = pysndfile.PySndfile(fname, 'r')
    if sndfile.channels() != 1:
        raise RuntimeError('please only load mono files')
    Fs = sndfile.samplerate()
    signal = sndfile.read_frames()
    pxx, freq, time = mlab.specgram(signal, NFFT=1024, noverlap=512, Fs=Fs)
    return pxx, freq, time


def main():
    files = [
            ("scattering: 0", "tunnel_near_no_scatter.wav"),
            ("scattering: 0.1", "tunnel_near.wav"),
            ]

    specgrams = [(label, get_specgram_data(fname)) for label, fname in files]
    fig, axes = plt.subplots(nrows=len(specgrams), sharex=True)

    cmap = plt.get_cmap('viridis')

    Z = map(lambda (label, (pxx, freq, time)): 10 * np.log10(pxx), specgrams)
    maxes = map(lambda z: np.nanmax(z), Z)
    print maxes

    vmin = -100
    vmax = max(maxes)

    for (label, (pxx, freq, time)), ax, z in zip(specgrams, axes, Z):
        im = ax.pcolormesh(time, freq, z, cmap=cmap, vmin=vmin, vmax=vmax, rasterized=True)
        ax.set_ylim(20, 20000)
        ax.set_yscale('log')

        ax.set_title(label)
        ax.set_xlabel('time / s')
        ax.set_ylabel('frequency / Hz')

        ax.set_xlim(0, 4)

    cb = fig.colorbar(im, ax=axes.ravel().tolist(), use_gridspec=True)
    cb.set_label('dB')

    plt.suptitle('Spectrograms of Tunnel Impulse Responses, With and Without Scattering')

    plt.tight_layout()
    plt.subplots_adjust(top=0.9)
    plt.subplots_adjust(right=0.75)

    plt.show()
    if render:
        plt.savefig('tunnel_spectrograms.svg', bbox_inches='tight', dpi=96, format='svg')

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
