# Goal:
# BRDF function which takes a dot product between a perfect and actual direction
# and a diffuseness coefficient, and which returns the volume in that direction.
# Should integrate to '1' over the valid output directions (i.e. a hemisphere).

import numpy as np
import matplotlib
render = False
if render:
    matplotlib.use('pgf')
import matplotlib.pyplot as plt
from string import split
import scipy.signal as sig
import wave
import math


def get_frac(numerator, denominator):
    return 0 if denominator == 0 or np.isnan(denominator) or np.isinf(
        denominator) else numerator / denominator


def brdf(y, d):
    y_sq = y * y
    one_minus_d_sq = pow(1 - d, 2)
    numerator = 2 * one_minus_d_sq * y_sq + 2 * d - 1

    if 0.5 <= d:
        denominator = 4 * np.pi * d * np.sqrt(one_minus_d_sq * y_sq + 2 * d - 1)
        extra = ((1 - d) * y) / (2 * np.pi * d)
        return get_frac(numerator, denominator) + extra

    denominator = 4 * np.pi * d * np.sqrt(one_minus_d_sq * y_sq + 2 * d - 1)
    return get_frac(numerator, denominator)


def main():
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'font.monospace': ['Input Mono Condensed'],
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    brdf_v = np.vectorize(brdf)

    angles = np.linspace(0, np.pi / 2, 1000)

    for i in np.linspace(0.1, 1, 10):
        plt.plot(angles, brdf_v(np.cos(angles), i))

    plt.show()


if __name__ == "__main__":
    main()
