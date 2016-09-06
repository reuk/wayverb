import argparse
import numpy as np
import pysndfile
import matplotlib
import matplotlib.pyplot as plt

SPEED_OF_SOUND = 340

def modal_analysis(fnames, max_frequency, room_dim=None):
    plt.figure()

    for fname in fnames:
        sndfile = pysndfile.PySndfile(fname, 'r')
        if sndfile.channels() != 1:
            raise RuntimeError('please only load mono files')
        n = sndfile.frames()
        sr = sndfile.samplerate()
        samples = sndfile.read_frames()
        fft = np.abs(np.fft.rfft(samples))
        freqs = np.fft.rfftfreq(n, d=1. / sr)
        mask = freqs < max_frequency
        fft = 20 * np.log10(fft[mask])
        freqs = freqs[mask]
        plt.plot(freqs, fft)

    if room_dim is not None:
        for dim in room_dim:
            frequency = SPEED_OF_SOUND / dim
            for frequency in np.arange(frequency, max_frequency, frequency):
                plt.axvline(frequency)

    plt.show()


def main():
    parser = argparse.ArgumentParser(
        description='do modal analysis on some set of files')
    parser.add_argument(
        'fnames',
        type=str,
        nargs='+',
        help='a list of files to analyse')
    parser.add_argument(
        '--room_dim',
        type=float,
        nargs=3,
        help='the dimensions of the room w/h/l')
    parser.add_argument(
        '--max_frequency',
        type=float,
        nargs=1,
        default=150,
        help='analyse up to this frequency')

    args = parser.parse_args()

    modal_analysis(args.fnames, args.max_frequency, args.room_dim)

if __name__ == '__main__':
    main()
