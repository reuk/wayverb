import numpy as np
import matplotlib
render = True
if render:
    matplotlib.use('pgf')
import matplotlib.pyplot as plt
from string import split
import math

def main():
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'font.monospace': ['Input Mono Condensed'],
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    shape = 0
    this_file = "omni.energies.txt"
    bands = 7

    plt.figure(figsize=(12, 6), dpi=100)

    energies_ = []
    max_vals_ = []

    for i in range(bands):
        band_nrg = []
        band_max = []
        with open(this_file) as f:
            for line in f:
                s = split(line)
                band_nrg.append(float(s[5 + i * 6]))
                band_max.append(float(s[7 + i * 6]))
        band_nrg.append(band_nrg[0])
        energies_.append(np.sqrt(band_nrg))
        max_vals_.append(band_max)

    for energies, i in zip(energies_, range(bands)):
        m = max(energies)

        theta = np.linspace(0, 2 * np.pi, len(energies), True)

        desired = [(1 - shape) + shape * np.cos(t) for t in theta]
        desired = [abs(d * m) for d in desired]

        errors = [np.abs((b - a) / b) for a, b in filter(lambda (x, y): y != 0, zip(energies, desired))]
        print "max error:", max(errors)
        print "mean error:", sum(errors) / len(errors)

        print i
        ax = plt.subplot(2, 4, 1 + i, projection='polar')
        ax.plot(theta, energies, color='r', linewidth=1)
        ax.plot(theta, desired, color='b', linewidth=1)

    plt.show()
    if render:
        plt.savefig(this_file + ".plot.pdf", bbox_inches="tight")

if __name__ == "__main__":
    main()
