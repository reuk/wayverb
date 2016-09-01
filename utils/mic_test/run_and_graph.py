from subprocess import call
from os.path import join
from os import environ
import numpy as np
import matplotlib
render = True
if render:
    matplotlib.use('pgf')
import matplotlib.pyplot as plt
from string import split
import math

project_path = "/Users/reuben/development/waveguide"

environ["GLOG_logtostderr"] = "1"

PATTERNS = [
    ("omni", 0),
    ("cardioid", 0.5),
    ("bidirectional", 1),
]

def run():
    exe = join(project_path, "build/utils/mic_test/mic_offset_rotate")
    out_dir = join(project_path, "utils/mic_test/output")

    for pattern, _ in PATTERNS:
        o_dir = join(out_dir, pattern)
        cmd_1 = ["mkdir", "-p", o_dir]
        cmd_2 = [exe, o_dir, pattern]
        print cmd_1
        call(cmd_1)
        print cmd_2
        call(cmd_2)

def graph():
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'font.monospace': ['Input Mono Condensed'],
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    base_folder = join(project_path, "tests/mic_test/output")

    bands = 1

    for this_file, shape in PATTERNS:
        this_file = base_folder + "/" + this_file + "/" + this_file + ".energies.txt"

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
            energies_.append(band_nrg)
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
            ax.set_xticklabels([])
            ax.set_yticklabels([])
            ax.plot(theta, energies, color='r', linewidth=1)
            ax.plot(theta, desired, color='b', linewidth=1)

        plt.show()
        if render:
            plt.savefig(this_file + ".plot.pdf", bbox_inches="tight")


def main():
    run()
    graph()

if __name__ == "__main__":
    main()
