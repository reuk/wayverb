import numpy as np
import matplotlib.pyplot as plt


def gaussian(t, bandwidth):
    return np.power(np.e, -np.power(t, 2.0) / np.power(bandwidth, 2.0))


def gaussian_kernel(sampling_frequency):
    upper = sampling_frequency / 4.0
    o = 2.0 / (np.pi * upper)
    l = int(np.ceil(8 * o * sampling_frequency) * 2)
    time_offset = l / (2.0 * sampling_frequency)
    ret = np.fromfunction(
        lambda x: gaussian(
            x / sampling_frequency - time_offset, o), (l,))
    return ret


def sin_modulated_gaussian(t, bandwidth, frequency):
    return -gaussian(t, bandwidth) * np.sin(2 * np.pi * frequency * t)


def sin_modulated_gaussian_kernel(sampling_frequency):
    upper = sampling_frequency / 4.0
    o = 2.0 / (np.pi * upper)
    l = int(np.ceil(8 * o * sampling_frequency) * 2)
    time_offset = l / (2.0 * sampling_frequency)
    ret = np.fromfunction(
        lambda x: sin_modulated_gaussian(
            x / sampling_frequency - time_offset, o, upper * 0.5), (l,))
    return ret


def ricker(t, f):
    u = np.pi * np.pi * f * f * t * t
    return (1.0 - 2.0 * u) * np.exp(-u)


def ricker_kernel(sampling_frequency):
    upper = sampling_frequency / 4.0
    o = 2.0 / (np.pi * upper)
    l = int(np.ceil(8 * o * sampling_frequency) * 2)
    time_offset = l / (2.0 * sampling_frequency)
    ret = np.fromfunction(
        lambda x: ricker(
            x / sampling_frequency - time_offset, upper * 0.5), (l,))
    return ret


def main():
    fig, ax = plt.subplots(2)

    sr = 16000.0

    def get_kernel(func):
        return np.pad(func(sr), 100, 'constant', constant_values=0)

    def plot_kernel(func):
        kernel = get_kernel(func)
        ax[0].plot(kernel, label=func.__name__)

        normalization_factor = np.real(np.fft.rfft(np.ones(kernel.size))[0])
        fd = np.fft.rfft(kernel) / normalization_factor
        n = len(kernel)
        freq = np.fft.rfftfreq(n, 1.0 / sr)

        ax[1].plot(freq, np.abs(fd), label=func.__name__)

    plot_kernel(gaussian_kernel)
    plot_kernel(sin_modulated_gaussian_kernel)
    plot_kernel(ricker_kernel)

    ax[0].legend()
    ax[0].set_xlabel('samples')
    ax[0].set_ylabel('amplitude')
    ax[0].set_title('time domain')

    ax[1].legend()
    ax[1].set_xlabel('frequency / Hz')
    ax[1].set_ylabel('normalized magnitude')
    ax[1].set_title('frequency domain')

    plt.legend()
    plt.show()


if __name__ == "__main__":
    main()
