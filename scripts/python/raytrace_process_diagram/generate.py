#!/usr/local/bin/python

import numpy as np
import matplotlib
render = True
if render:
    matplotlib.use('pgf')
import matplotlib.pyplot as plt


def a2db(a):
    return 20 * np.log10(a)


################################################################################


def constant_mean_event_occurrence(speed_of_sound, room_volume):
    return 4 * np.pi * np.power(speed_of_sound, 3.0) / room_volume


def mean_event_occurrence(constant, t):
    return min(constant * np.square(t), 10000.0)


def compute_t0(constant):
    return np.power(2.0 * np.log(2.0) / constant, 1.0 / 3.0)


def interval_size(mean_occurrence):
    return np.log(1.0 / (1.0 - np.random.random_sample())) / mean_occurrence


def generate_dirac_sequence(speed_of_sound, room_volume, sample_rate, max_time):
    constant = constant_mean_event_occurrence(speed_of_sound, room_volume)
    ret = np.zeros(int(np.ceil(max_time * sample_rate)))
    t = compute_t0(constant)
    while t < max_time:
        sample_index = t * sample_rate
        twice = int(2 * sample_index)
        negative = (twice % 2) != 0
        ret[int(sample_index)] = -1.0 if negative else 1.0

        t += interval_size(mean_event_occurrence(constant, t))

    return ret


def gen_histogram(steps):
    factors = np.power(10.0, -6.0 / steps)
    max_steps = int(np.max(steps))
    ind = np.arange(max_steps)
    return np.transpose(np.tile(factors, (max_steps, 1))) ** ind


def weight_sequence(histogram, histogram_sr, sequence, sequence_sr, acoustic_impedance):
    def convert_index(ind):
        return int(ind * sequence_sr / histogram_sr)

    bands = np.shape(histogram)[0]
    histogram_steps = np.shape(histogram)[1]
    
    ideal_sequence_length = convert_index(histogram_steps)
    seq = np.resize(sequence, min(ideal_sequence_length, len(sequence)))
    ret = np.tile(seq, (bands, 1))

    for i in range(histogram_steps):
        beg = convert_index(i)
        end = convert_index(i + 1)
        squared_summed = np.sum(np.square(sequence[beg:end]))
        scale_factor = histogram[:, i] / squared_summed if squared_summed != 0 else 0
        scale_factor = np.sqrt(scale_factor * acoustic_impedance)
        ret[:, beg:end] *= np.transpose(np.tile(scale_factor, (end - beg, 1)))

    return ret


################################################################################


def max_width_factor(min, max, step):
    base = np.power(max / min, step)
    return (base - 1) / (base + 1)


def width_factor(min, max, bands, overlap):
    return max_width_factor(min, max, 1.0 / bands) * overlap


def band_edge_impl(p, P, l):
    return ((p / P) + 1) / 2 if l == 0 else np.sin(np.pi * band_edge_impl(p, P, l - 1) / 2) 


def lower_band_edge(p, P, l):
    return np.where(p < -P, 0.0, np.where(p < P, np.square(np.sin(np.pi * band_edge_impl(p, P, l) / 2.0)), 1.0))


def upper_band_edge(p, P, l):
    return np.where(p < -P, 1.0, np.where(p < P, np.square(np.cos(np.pi * band_edge_impl(p, P, l) / 2.0)), 0.0))


def band_edge_frequency(band, bands, min, max):
    return min * np.power(max / min, band / float(bands))


def compute_lopass_magnitude(frequency, edge, width_factor, l):
    return upper_band_edge(frequency - edge, edge * width_factor, l)


def compute_hipass_magnitude(frequency, edge, width_factor, l):
    return lower_band_edge(frequency - edge, edge * width_factor, l)


def compute_bandpass_magnitude(frequency, min, max, width_factor, l):
    return compute_lopass_magnitude(frequency, max, width_factor, l) * compute_hipass_magnitude(frequency, min, width_factor, l)


################################################################################


def main():
    plt.figure(figsize=(10, 12))
    plt.suptitle('Overview of Conversion Process for Ray Traced Energy Histograms')

    plots_x = 2
    plots_y = 4

    ax = None

    speed_of_sound = 340.0
    acoustic_impedance = 400.0
    room_volume = 10000.0
    output_sample_rate = 44100.0
    rt60 = 0.4

    signal_length = rt60

    # Generate and plot sequence

    sequence = generate_dirac_sequence(speed_of_sound, room_volume, output_sample_rate, signal_length)
    times = np.arange(len(sequence)) / output_sample_rate

    ax = plt.subplot(plots_y, plots_x, 1)
    ax.set_title('1. Poisson Dirac Sequence')
    ax.set_xlabel('time / s')
    ax.set_ylabel('amplitude')
    ax.plot(times, sequence)

    # Weight sequence by histograms

    histogram_sr = 1000.0
    histogram_steps = histogram_sr * signal_length

    bands = 8

    histogram = gen_histogram(np.linspace(histogram_steps, histogram_steps / 2, bands)) * 0.004

    weighted = weight_sequence(histogram, histogram_sr, sequence, output_sample_rate, acoustic_impedance)
    ax = plt.subplot(plots_y, plots_x, 2)
    ax.set_title('2. Per-band Weighted Sequences')
    ax.set_xlabel('time / s')
    ax.set_ylabel('amplitude')
    for i in weighted:
        ax.plot(times, i)

    # Plot weighted sequences in frequency domain

    ax = plt.subplot(plots_y, plots_x, 3)
    ax.set_title('3. Weighted Sequences in the Frequency Domain')
    ax.set_xscale('log')
    ax.set_xlabel('frequency / Hz')
    ax.set_ylabel('modulus / dB')
    for i in weighted:
        frequency_domain = np.fft.rfft(i)
        frequencies = np.fft.rfftfreq(len(i)) * output_sample_rate
        ax.plot(frequencies, a2db(np.abs(frequency_domain) / len(i)))

    # Plot filtered sequences in frequency domain

    minf = 20.0 / output_sample_rate
    maxf = 20000.0 / output_sample_rate

    band_edges = band_edge_frequency(np.arange(bands + 1), bands, minf, maxf)
    lower_edges = band_edges[:-1]
    upper_edges = band_edges[1:]

    ax = plt.subplot(plots_y, plots_x, 4)
    ax.set_title('4. Filtered Sequences in the Frequency Domain')
    ax.set_xscale('log')
    ax.set_xlabel('frequency / Hz')
    ax.set_ylabel('modulus / dB')

    wf = width_factor(minf, maxf, bands, 1.0)

    frequencies = np.fft.rfftfreq(len(sequence))

    ffts = np.fft.rfft(weighted, axis=1)
    for i in range(bands):
        ffts[i] *= compute_bandpass_magnitude(frequencies, lower_edges[i], upper_edges[i], wf, 0)

    for i in ffts:
        ax.plot(frequencies * output_sample_rate, a2db(np.abs(i) / len(frequencies)))

    # Plot filtered sequences in time domain

    ax = plt.subplot(plots_y, plots_x, 5)
    ax.set_title('5. Filtered Sequences in the Time Domain')
    ax.set_xlabel('time / s')
    ax.set_ylabel('amplitude')

    iffts = np.fft.irfft(ffts, axis=1)
    for i in reversed(iffts):
        ax.plot(times, i)

    # Plot final output

    final_output = np.sum(iffts, axis=0)

    ax = plt.subplot(plots_y, plots_x, 6)
    ax.set_title('6. Summed Bands in the Time Domain')
    ax.set_xlabel('time / s')
    ax.set_ylabel('amplitude')
    ax.plot(times, final_output)

    # Plot spectrogram

    ax = plt.subplot(plots_y, 1, plots_y)
    ax.set_title('7. Spectrogram of Broadband Signal')
    ax.set_xlabel('time / s')
    ax.set_ylabel('frequency / Hz')
    Pxx, freqs, bins, im = plt.specgram(final_output, NFFT=1024, Fs=output_sample_rate, noverlap=512)

    plt.tight_layout()
    plt.subplots_adjust(top=0.9)

    plt.show()
    if render:
        plt.savefig(
            'raytrace_process.svg',
            bbox_inches='tight',
            dpi=300,
            format='svg')


if __name__ == '__main__':
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'font.monospace': ['Input Mono Condensed'],
        'legend.fontsize': 12,
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    main()
