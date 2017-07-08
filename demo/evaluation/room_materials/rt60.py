#!/usr/local/bin/python

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
import os
import re
import json

import sys
sys.path.append('python')


def get_frequency_rt30_tuple(line):
    split = line.split()
    return (float(split[0]), float(split[6]))


def read_rt30(fname):
    with open(fname) as f:
        lines = f.readlines()

    return [get_frequency_rt30_tuple(line) for line in lines[14:22]]


def main():
    files = [
            ("0.02", "0.02.txt"),
            ("0.04", "0.04.txt"),
            ("0.08", "0.08.txt"),
            ]

    for label, fname in files:
        tuples = read_rt30(fname)

        x = [freq for freq, _ in tuples]
        y = [time for _, time in tuples]

        min_time = min(y)
        max_time = max(y)

        average = (max_time - min_time) * 100.0 / ((max_time + min_time) * 0.5)

        print('file: {}, min: {}, max: {}, average: {}'.format(
            fname, min_time, max_time, average))

        plt.plot(x, y, label=label, marker='o', linestyle='--')

    plt.xscale('log')

    plt.axvline(x=500)

    plt.annotate(xy=(520, 1.4), s='waveguide cutoff')

    plt.legend(loc='lower center', ncol=3, bbox_to_anchor=(0, -0.05, 1, 1), bbox_transform=plt.gcf().transFigure)
    plt.title('Octave-band T30 Measurements for Different Surface Absorption Coefficients')

    plt.xlabel('frequency / Hz')
    plt.ylabel('time / s')

    plt.tight_layout()
    #plt.subplots_adjust(top=0.9)

    plt.show()
    if render:
        plt.savefig('room_absorption_rt30.svg', bbox_inches='tight', dpi=96, format='svg')


if __name__ == '__main__':
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'legend.fontsize': 12,
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    main()
