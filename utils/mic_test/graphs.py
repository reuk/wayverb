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

def main():
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'font.monospace': ['Input Mono Condensed'],
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    subdirs = [
        root for root,
        subdirs,
        _ in os.walk(out_dir) if not subdirs]

    for subdir in subdirs:
        plt.figure(figsize=(12, 6), dpi=100)

        with open(os.path.join(subdir, "energies.txt")) as f:
            obj = json.load(f)

        shape = obj["directionality"]

        bands = 8
        energies = [[struct["energy"]["value" + str(i)] for struct in obj["energies"]]
                    for i in range(bands)]

        for nrg in energies:
            nrg.append(nrg[0])

        for nrg, i in zip(energies, range(bands)):
            m = max(nrg)

            theta = np.linspace(0, 2 * np.pi, len(nrg), True)

            desired = [(1 - shape) + shape * np.cos(t) for t in theta]
            desired = [abs(d * m) for d in desired]

            errors = [np.abs((b - a) / b) for a, b in filter(lambda (x, y): y != 0, zip(nrg, desired))]

            ax = plt.subplot(2, 4, 1 + i, projection='polar')
            ax.set_xticklabels([])
            ax.set_yticklabels([])
            ax.plot(theta, nrg, color='r', linewidth=1)
            ax.plot(theta, desired, color='b', linewidth=1)

        plt.show()
        if render:
            plt.savefig(os.path.join(subdir, "plot.pdf"), bbox_inches="tight")

if __name__ == "__main__":
    main()
