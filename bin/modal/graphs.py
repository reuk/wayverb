#!/usr/local/bin/python

import numpy as np
import matplotlib
render = True
if render:
    matplotlib.use('pgf')
import matplotlib.pyplot as plt
import pysndfile
from string import split
import scipy.signal as signal
import wave
import math
import os
import re
import json
import itertools


SPEED_OF_SOUND = 340


def do_plot(ax, image_source_file, waveguide_file, absorption, max_frequency, room_dim):
    ax.set_title('Absorption: ' + str(absorption))

    if room_dim is not None:
        ranges = [[(x / i) ** 2 for x in range(10)] for i in room_dim]
        all_frequencies = [(SPEED_OF_SOUND / 2) * np.sqrt(a + b + c)
                           for a, b, c in itertools.product(*ranges)]
        filtered_frequencies = [i for i in all_frequencies if i < max_frequency]
        for f in filtered_frequencies:
            plt.axvline(f, color="0.75")

    for fname, label in [(image_source_file, 'image source'), (waveguide_file, 'waveguide')]:
        sndfile = pysndfile.PySndfile(fname, 'r')
        if sndfile.channels() != 1:
            raise RuntimeError('please only load mono files')
        n = sndfile.frames()
        sr = sndfile.samplerate()
        samples = sndfile.read_frames()
        fft = np.abs(np.fft.rfft(samples))
        freqs = np.fft.rfftfreq(n, d=1. / sr)
        mask = freqs < max_frequency
        fft = 20 * np.log10(fft[mask])
        freqs = freqs[mask]
        plt.plot(freqs, fft, label=label)


def main():
    root_dir = '/Users/reuben/development/waveguide/build/bin/siltanen2013'

    fnames = [os.path.join(root_dir, i) for i in [
        'a_0.05_null.img_src.aif',
        'a_0.05_null.waveguide.single_band.aif',

        'a_0.1_null.img_src.aif',
        'a_0.1_null.waveguide.single_band.aif',

        'a_0.2_null.img_src.aif',
        'a_0.2_null.waveguide.single_band.aif',
    ]]

    max_frequency = 200 
    room_dim = [5.56, 3.97, 2.81]

    plt.suptitle('Comparison of Image Source and Calibrated Waveguide Frequency Responses')

    do_plot(
        plt.subplot(3, 1, 1),
        fnames[0],
        fnames[1],
        0.05,
        max_frequency,
        room_dim)

    do_plot(
        plt.subplot(3, 1, 2),
        fnames[2],
        fnames[3],
        0.1,
        max_frequency,
        room_dim)

    do_plot(
        plt.subplot(3, 1, 3),
        fnames[4],
        fnames[5],
        0.2,
        max_frequency,
        room_dim)

    plt.legend(loc='lower center', ncol=2, bbox_to_anchor=(0, -0.05, 1, 1), bbox_transform=plt.gcf().transFigure)

    plt.tight_layout()
    plt.subplots_adjust(top=0.9)

    plt.show()

    plt.show()
    if render:
        plt.savefig('calibration.svg', bbox_inches='tight', dpi=300, format='svg')
    

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
