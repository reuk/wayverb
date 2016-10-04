#!/usr/local/bin/python

import numpy as np
import pysndfile
import itertools
import os.path
import time
from functools import reduce

import time


class Timer:

    def __enter__(self):
        self.start = time.clock()
        return self

    def __exit__(self, *args):
        self.end = time.clock()
        self.interval = self.end - self.start


def get_fft(fname):
    samples, _, _ = pysndfile.sndio.read(fname)
    # seems to be hella slow if fft input signal size has a large prime factor
    samples = np.resize(samples, 88200 * 30)
    print "loaded", len(samples), "samples"
    print "running fft..."
    with Timer() as t:
        ret = np.fft.rfft(samples)
    print "done in", t.interval, "seconds"
    return ret


def main():
    samples = np.zeros((8,))
    samples[1] = 1
    fft = np.fft.rfft(samples)
    print fft
    

    fnames = ['/Users/reuben/development/waveguide/demo/assets/noise.wav',
              '/Users/reuben/development/waveguide/demo/assets/sweep.wav']
    ffts = [get_fft(i) for i in fnames]

    reduced = reduce(lambda i, j: i + j, ffts)

    reduced *= 0.5

    output = np.fft.irfft(reduced)
    sndfile = pysndfile.sndio.write('summed_ffts.wav', output, 88200)


if __name__ == '__main__':
    main()
