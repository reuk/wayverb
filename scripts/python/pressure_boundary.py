#!/usr/local/bin/python

import numpy as np
import pysndfile
import itertools
import os.path


def main():
    a = 0.1
    r = np.sqrt(1 - a)
    specific_impedance = (1 + r) / (1 - r)

    def r_for_angle(angle):
        return ((specific_impedance * np.cos(angle) - 1) /
                (specific_impedance * np.cos(angle) + 1))



if __name__ == '__main__':
    main()
