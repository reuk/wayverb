#!/usr/local/bin/python

from subprocess import call
import os
import json
import numpy as np
import matplotlib
render = True
if render:
    matplotlib.use('pgf')
import matplotlib.pyplot as plt
from string import split
import math

from paths import *

DPI=96

def do_plot(subdir, fname):
    plt.figure(figsize=(20, 10), dpi=DPI)

    with open(os.path.join(subdir, fname)) as f:
        obj = json.load(f)

    shape = obj["directionality"]

    name = {0.0: 'Omnidirectional',
            0.5: 'Cardioid',
            1.0: 'Bidirectional',
           }[shape]

    normalised_band_edges = [0.01, 0.0141421, 0.02, 0.0282843, 0.04, 0.0565685, 0.08, 0.113137, 0.16]

    sr = 50000
    band_edges = [i * sr for i in normalised_band_edges]

    bands = len(obj["energies"][0]["energy"])
    energies = [[(struct["angle"], struct["energy"]["value" + str(i)])
                 for struct in obj["energies"]]
                for i in range(bands)]

    for nrg in energies:
        nrg.append(nrg[0])

    normalisation_factors = [data[0][1] for data in energies]

    energies = [[(_, t / factor) for _, t in data]
                for data, factor in zip(energies, normalisation_factors)]

    the_max = max([max(data, key=lambda (_, t): t)[1] for data in energies])

    for data, i, lower, upper in zip(energies, range(bands), band_edges[:-1], band_edges[1:]):
        nrg = [t for _, t in data]

        theta = [t for t, _ in data]

        desired = [(1 - shape) + shape * np.cos(t) for t in theta]
        desired = [abs(d) for d in desired]

        ax = plt.subplot(2, 4, 1 + i, projection='polar')
        ax.set_ylim([0, the_max])

        ax.set_yticks(ax.get_yticks()[1::2])

        ax.plot(theta, nrg, color='r', linewidth=1)
        ax.plot(theta, desired, color='b', linewidth=1)

        ax.set_title(str(lower) + ' to ' + str(upper) + ' Hz')
        ax.title.set_position([.5, 1.1])

    plt.suptitle('Directional Response of ' + name + ' Receiver')
    #plt.tight_layout()
    plt.subplots_adjust(top=0.9)

    plt.show()
    if render:
        plt.savefig(
            name + '_response.svg',
            bbox_inches='tight',
            dpi=DPI,
            format='svg')


def main():
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'font.monospace': ['Input Mono Condensed'],
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    subdirs = [root for root, subdirs, _ in os.walk(out_dir) if not subdirs]

    for subdir in subdirs:
        files = [f for f in os.listdir(subdir) if f.endswith(".txt")]
        for f in files:
            do_plot(subdir, f)

if __name__ == "__main__":
    main()
