#!/usr/local/bin/python

import numpy as np
import scipy.signal
import matplotlib
render = True
if render:
    matplotlib.use('pgf')
import matplotlib.pyplot as plt


def gaussian(t, o):
    return np.exp(-np.square(t) / (2.0 * np.square(o)))


def sin_modulated_gaussian(t, o):
    return -gaussian(t, o) * np.sin(t / o)


def gaussian_dash(t, o):
    return -t * gaussian(t, o) / np.square(o)


def gauss_like_kernel(fc, func):
    o = 1.0 / (2.0 * np.pi * fc)
    delay = int(np.ceil(8.0 * o))
    return func(np.arange(delay * 2 + 1) - delay, o)


################################################################################


def ricker(t, f):
    u = np.square(np.pi * f * t)
    return (1.0 - 2.0 * u) * np.exp(-u)


def gen_ricker(fc):
    delay = int(np.ceil(1.0 / fc))
    return ricker(np.arange(delay * 2 + 1) - delay, fc)


################################################################################


def normalize(arr):
    return arr / np.max(np.abs(arr))


def factdbl(t):
    out = 1
    for i in range(t, 1, -2):
        out *= i
    return out


def maxflat(f0, N, A):
    h = np.zeros(4 * N - 1)
    Q = 2 * N - 1
    for n in range(-Q, Q + 1):
        top = (factdbl(Q)**2) * np.sin(n * 2.0 * np.pi * f0)
        hi = factdbl(2 * N + n - 1)
        lo = factdbl(2 * N - n - 1)
        coeff = 2.0 if (n % 2) != 0 else np.pi
        bot = n * hi * lo * coeff if n != 0 else 1.0
        h[n + Q] = top / bot

    h[Q] = 2.0 * f0

    h = normalize(h)
    h *= A
    return h


def compute_g0(acoustic_impedance, speed_of_sound, sample_rate, radius):
    courant_squared = 1.0 / 3
    ambient_density = acoustic_impedance / speed_of_sound
    sphere_surface_area = 4 * np.pi * radius * radius
    spatial_sample_period = speed_of_sound * np.sqrt(1 / courant_squared) / sample_rate
    return courant_squared * ambient_density * sphere_surface_area / spatial_sample_period


def mech_sphere(M, f0, Q, T):
    fs = 1.0 / T
    w0 = 2.0 * np.pi * f0 * fs
    K = M * np.square(w0)
    R = w0 * M / Q
    beta = w0 / np.tan(w0 * T / 2.0)
    den = M * np.square(beta) + R * beta + K
    b0 = beta / den
    b2 = -b0
    a1 = (2.0 * (K - M * np.square(beta))) / den
    a2 = 1.0 - (2.0 * R * beta / den)
    return (b0, 0, b2), (1, a1, a2)


def design_pcs_source(sample_rate, 
                      sphere_mass, 
                      low_cutoff_hz, 
                      low_q):
    pulse_shaping_filter = maxflat(0.2, 16, 0.00025)

    padded = np.zeros(300)
    padded[:len(pulse_shaping_filter)] = pulse_shaping_filter

    b, a = mech_sphere(sphere_mass, low_cutoff_hz / sample_rate, low_q, 1.0 / sample_rate)
    pulse_shaping_filter = scipy.signal.lfilter(b, a, padded)
    one_over_two_t = sample_rate / 2.0
    pulse_shaping_filter = scipy.signal.lfilter((one_over_two_t, 0, -one_over_two_t), (1,), pulse_shaping_filter)
    return pulse_shaping_filter


################################################################################


def main():
    plt.figure(figsize=(10, 12))
    plt.suptitle('Pulses With No DC Component')

    fc = 0.2
    oversample = 10

    sigs = [('Sine-modulated Gaussian', gauss_like_kernel(fc / 2.0, sin_modulated_gaussian)),
            ('Differentiated Gaussian', gauss_like_kernel(fc / 2.0, gaussian_dash)),
            ('Ricker', gen_ricker(fc / 2.0)),
            ('PCS', design_pcs_source(10000.0, 0.025, 200.0, 0.7)),
           ]

    plots_x = 2 # time-domain, frequency-domain
    plots_y = len(sigs) # sine-modulated gaussian, differentiated gaussian, ricker, PCS


    for i, (name, sig) in enumerate(sigs, start=0):
        ax = plt.subplot(plots_y, plots_x, i * 2 + 1)
        ax.set_title(name + ': time-domain')
        ax.set_xlabel('time / samples')
        ax.set_ylabel('amplitude')
        ax.plot(normalize(sig))

        #padded = np.zeros(len(sig))
        #resampled = sig[::oversample] 
        #padded[:len(resampled)] = resampled
        padded = np.zeros(4096)
        padded[:len(sig)] = sig

        ax = plt.subplot(plots_y, plots_x, i * 2 + 2)
        ax.set_title(name + ': frequency-domain')
        ax.set_xlabel('normalised frequency')
        ax.set_ylabel('magnitude')
        ax.plot(np.fft.rfftfreq(len(padded)), np.abs(np.fft.rfft(padded)))
        ax.axvline(fc)

    plt.tight_layout()
    plt.subplots_adjust(top=0.9)

    plt.show()
    if render:
        plt.savefig(
            'kernel_properties.svg',
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
