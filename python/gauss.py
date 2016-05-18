import numpy as np
import matplotlib.pyplot as plt


def gaussian(t, t0, bandwidth):
    return np.power(np.e, -np.power(t - t0, 2.0) / np.power(bandwidth, 2.0))


def sin_modulated_gaussian(t, t0, bandwidth, frequency):
    return -gaussian(t, t0, bandwidth) * \
        np.sin(2 * np.pi * frequency * (t - t0))


def waveguide_kernel(sampling_frequency):
    upper = sampling_frequency / 4.0
    o = 2.0 / (np.pi * upper)
    l = int(np.ceil(8 * o * sampling_frequency) * 2)
    time_offset = l / (2.0 * sampling_frequency)
    ret = np.fromfunction(lambda x: sin_modulated_gaussian(
        x / sampling_frequency, time_offset, o, upper * 0.5), (l,))
    return ret


def main():
    fig, ax = plt.subplots(2)

    sr = 16000.0

    #kernel = np.pad(waveguide_kernel(sr), 100, 'constant', constant_values=0)
    kernel = np.pad(np.ones(1), 100, 'constant', constant_values=0)

    ax[0].plot(kernel)
    ax[0].set_xlabel('samples')
    ax[0].set_ylabel('amplitude')
    ax[0].set_title('time domain')

    normalization_factor = np.real(np.fft.rfft(np.ones(kernel.size))[0])
    #normalization_factor = 1

    fd = np.fft.rfft(kernel) / normalization_factor
    n = len(kernel)
    freq = np.fft.rfftfreq(n, 1.0 / sr)

    ax[1].plot(freq, np.abs(fd))
    ax[1].set_xlabel('frequency / Hz')
    ax[1].set_ylabel('normalized magnitude')
    ax[1].set_title('frequency domain')

    plt.show()


if __name__ == "__main__":
    main()
