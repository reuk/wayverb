import numpy as np
import matplotlib.pyplot as plt
import scipy.signal
import scipy.stats


def block_dc(sig, a=0.99):
    return scipy.signal.filtfilt([1, -1], [1, -a], sig)


def main():
    sample_rate = 44100.0

    length = sample_rate * 10
    t = np.arange(length)

    increasing = (np.random.rand(length) * 2 - 1) * np.exp(np.linspace(0, -60, length)) + 2 * np.exp(np.linspace(0, 10, length))
    increasing_filtered = block_dc(increasing)

    a, b, r, _, _ = scipy.stats.linregress(t, np.log(increasing))
    print "a:", a, "b:", b, "r:", r
    correction_curve = np.exp(b) * np.exp(t * a)
    increasing_corrected = increasing - correction_curve

    plt.figure()
    plt.plot(t, increasing_filtered, label="filtered")
    plt.plot(t, increasing_corrected, label="corrected")
    plt.legend()
    plt.show()

if __name__ == "__main__":
    main()
