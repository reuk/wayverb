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
sys.path.append("python")

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

    plt.ylabel('Amplitude / dB')
    plt.xlabel('Normalized Frequency')

    if USE_DB_AXES:
        plt.plot(w, a2db(np.abs(h)), label=label)
    else:
        plt.plot(w, np.abs(h), label=label)
    plt.axvline(CUTOFF)


def boundary_coefficient_plot(num, den, azimuth, elevation, label):
    num, den = compute_boundary_coefficients(num, den, azimuth, elevation)
    frequency_plot(num, den, label)


def show_graph(free_field_file, subbed_file, reflectance, impedance, azimuth, elevation):
    files = [("free", free_field_file), ("reflection response", subbed_file), ]

    def get_signals(f):
        spf = wave.open(f)
        signal = spf.readframes(-1)
        return np.fromstring(signal, "Int16")

    signals = [(tag, get_signals(i)) for tag, i in files]

    n = signals[0][1].size

    ffts = [(tag, np.fft.rfft(i)) for tag, i in signals]
    freq = np.fft.rfftfreq(n)

    def do_plot(tag, a):
        plt.plot(np.resize(freq, n / 4), np.resize(a, n / 4), label=tag)

    div = np.abs(ffts[1][1] / ffts[0][1])
    if USE_DB_AXES:
        div = a2db(div)
    do_plot("divided", div)

    def bcp((b, a), az, el, label):
        boundary_coefficient_plot(b, a, az, el, label)

    bcp(reflectance, azimuth, elevation, "reflectance")
    bcp(impedance, azimuth, elevation, "impedance")

    plt.legend()

def extract_filter_coefficients(item):
    b = [item["b"]["value" + str(i)] for i in range(len(item["b"]))]
    a = [item["a"]["value" + str(i)] for i in range(len(item["a"]))]
    return b, a

def main():
    suffix_free = "_windowed_free_field.wav"
    suffix_subb = "_windowed_subbed.wav"

    info_file = "coefficients.txt"

    subdirs = [
        root for root,
        subdirs,
        _ in os.walk(out_dir) if not subdirs]

    matcher = re.compile("az_([0-9]+\.[0-9]+)_el_([0-9]+\.[0-9]+)")
    for subdir in subdirs:
        plt.figure(figsize=(8.27, 11.69))

        groups = matcher.match(os.path.basename(subdir)).groups()
        azimuth = float(groups[0])
        elevation = float(groups[1])

        with open(os.path.join(subdir, info_file)) as f:
            obj = json.load(f)

        num_subplots = len(obj["coefficients"])
        for item, subplot in zip(obj["coefficients"], range(num_subplots)):
            b, a = extract_filter_coefficients(item["impedance_coefficients"])

            ax = plt.subplot(num_subplots, 1, subplot + 1)
            ax.set_title(item["name"])
            show_graph(os.path.join(subdir, item["name"] + suffix_free),
                       os.path.join(subdir, item["name"] + suffix_subb),
                       extract_filter_coefficients(item["reflectance_coefficients"]),
                       extract_filter_coefficients(item["impedance_coefficients"]),
                       azimuth,
                       elevation)

        plt.tight_layout()
        plt.show()
        if render:
            plt.savefig(
                os.path.join(
                    subdir,
                    "plot.pdf"),
                bbox_inches="tight",
                dpi=300)


if __name__ == "__main__":
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'font.monospace': ['Input Mono Condensed'],
        'legend.fontsize': 7,
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    main()
