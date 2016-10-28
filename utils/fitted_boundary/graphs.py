#!/usr/local/bin/python

import numpy as np
import matplotlib
render = True
if render:
    matplotlib.use('pgf')
import matplotlib.pyplot as plt
import scipy.signal as signal
import json
import os.path

from paths import *

USE_DB_AXES = True
CUTOFF = 0.196

def a2db(a):
    return 20 * np.log10(a)


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
    

def extract_filter_coefficients(item):
    b = [item["b"]["value" + str(i)] for i in range(len(item["b"]))]
    a = [item["a"]["value" + str(i)] for i in range(len(item["a"]))]
    return b, a


def main():
    info_file = os.path.join(out_dir, "coefficients.json")
    with open(info_file) as f:
        obj = json.load(f)

    root = obj["value0"]
    num_subplots = len(root)

    for item, subplot in zip(root, range(num_subplots)):
        key, value = item.popitem()
        print key
        print value
        ax = plt.subplot(num_subplots, 1, subplot + 1)
        ax.set_title(key)

        for tag in ["reflectance", "impedance"]:
            num, den = extract_filter_coefficients(value[tag])
            frequency_plot(num, den, tag)

        plt.legend()

    plt.tight_layout()
    plt.show()
    if render:
        plt.savefig(os.path.join(out_dir, "plot.pdf"),
                    bbox_inches="tight",
                    dpi=300)

if __name__ == "__main__":
    main()
