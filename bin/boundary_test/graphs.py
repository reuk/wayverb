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

from paths import *

import sys
sys.path.append('python')

USE_DB_AXES = True

CUTOFF = 0.196


def a2db(a):
    return 20 * np.log10(a)


def compute_boundary_coefficients(b_imp, a_imp, azimuth, elevation):
    ca = np.cos(azimuth)
    ce = np.cos(elevation)

    num = [b * ca * ce - a for b, a in zip(b_imp, a_imp)]
    den = [b * ca * ce + a for b, a in zip(b_imp, a_imp)]

    a0 = den[0]

    num = [i / a0 for i in num]
    den = [i / a0 for i in den]

    return num, den


def frequency_plot(num, den, label):
    w, h = signal.freqz(num, den)
    n = len(w)
    w.resize(n / 2)
    h.resize(n / 2)
    w /= np.pi * 2

    plt.ylabel('amplitude / dB')
    plt.xlabel('normalised frequency')

    if USE_DB_AXES:
        plt.plot(w, a2db(np.abs(h)), label=label, linestyle='dashed')
    else:
        plt.plot(w, np.abs(h), label=label)
    plt.axvline(CUTOFF)


def boundary_coefficient_plot(num, den, azimuth, elevation, label):
    num, den = compute_boundary_coefficients(num, den, azimuth, elevation)
    frequency_plot(num, den, label)


def show_graph(free_field_file, subbed_file, reflectance, impedance, azimuth, elevation):
    files = [('free', free_field_file), ('reflection response', subbed_file), ]

    def get_signals(f):
        spf = wave.open(f)
        signal = spf.readframes(-1)
        return np.fromstring(signal, 'Int16')

    signals = [(tag, get_signals(i)) for tag, i in files]

    n = signals[0][1].size

    ffts = [(tag, np.fft.rfft(i)) for tag, i in signals]
    freq = np.fft.rfftfreq(n)

    def do_plot(tag, a):
        plt.plot(np.resize(freq, n / 4), np.resize(a, n / 4), label=tag)

    div = np.abs(ffts[1][1] / ffts[0][1])
    if USE_DB_AXES:
        div = a2db(div)
    do_plot('measured', div)

    def bcp((b, a), az, el, label):
        boundary_coefficient_plot(b, a, az, el, label)

    # bcp(reflectance, azimuth, elevation, 'reflectance')
    bcp(impedance, azimuth, elevation, 'predicted')

def extract_filter_coefficients(item):
    b = [item['b']['value' + str(i)] for i in range(len(item['b']))]
    a = [item['a']['value' + str(i)] for i in range(len(item['a']))]
    return b, a

def main():
    suffix_free = '_windowed_free_field.wav'
    suffix_subb = '_windowed_subbed.wav'

    info_file = 'coefficients.txt'

    matcher = re.compile('az_([0-9]+\.[0-9]+)_el_([0-9]+\.[0-9]+)')

    plt.figure(figsize=(10, 10))

    # load file
    with open(os.path.join(out_dir, info_file)) as f:
        obj = json.load(f)

    fdata = {}
    for key in obj:
        fdata[obj[key]['material']] = {'tests':[]}

    for key in obj:
        value = obj[key]
        fdata[value['material']]['reflectance'] = extract_filter_coefficients(value['reflectance'])
        fdata[value['material']]['impedance'] = extract_filter_coefficients(value['impedance'])
        fdata[value['material']]['tests'].append((value['test'],
                                              value['az_el']['azimuth'],
                                              value['az_el']['elevation']))

    for key in fdata:
        fdata[key]['tests'] = sorted(fdata[key]['tests'], key=lambda (a, i, b): i)

    num_materials = len(fdata)
    num_plots = reduce(lambda a, b: max(a, len(fdata[b]['tests'])), fdata, 0)

    ax = None
    for material, key in enumerate(fdata, start=1):
        for trial, (test_name, azimuth, elevation) in enumerate(fdata[key]['tests']):
            ax = plt.subplot(num_plots, num_materials, material + trial * num_materials, sharex=ax, sharey=ax)

            az_deg = int(azimuth * 180 / np.pi)
            el_deg = int(elevation * 180 / np.pi)
            ax.set_title(key + ', az: ' + str(az_deg) + ', el: ' + str(el_deg))

            show_graph(os.path.join(out_dir, test_name + suffix_free),
                       os.path.join(out_dir, test_name + suffix_subb),
                       fdata[key]['reflectance'],
                       fdata[key]['impedance'],
                       azimuth,
                       elevation)

    plt.legend(loc='lower center', ncol=2, bbox_to_anchor=(0, -0.05, 1, 1), bbox_transform=plt.gcf().transFigure)
    plt.suptitle('Comparison of Measured and Predicted Boundary Reflectance')

    plt.tight_layout()
    plt.subplots_adjust(top=0.9)

    plt.show()
    if render:
        plt.savefig(
            os.path.join(out_dir, 'boundary_response.svg'),
            bbox_inches='tight',
            dpi=96,
            format='svg')


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
