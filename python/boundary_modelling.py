from math import pi, sin, cos, tan, sqrt
import numpy as np
import scipy.signal as signal
import matplotlib.pyplot as plt


def series_coeffs(c):
    return reduce(lambda (a, b), (x, y): (np.convolve(a, x), np.convolve(b, y)), c)

def twopass_coeffs(c):
    return series_coeffs(c + c)

def get_linkwitz_riley_coeffs(gain, lo, hi, sr):
    def get_c(cutoff, sr):
        wcT = pi * cutoff / sr
        return 1 / tan(wcT)

    def get_lopass_coeffs(gain, cutoff, sr):
        c = get_c(cutoff, sr)
        a0 = c * c + c * sqrt(2) + 1
        b = [gain / a0, 2 * gain / a0,           gain / a0]
        a = [1,         (-2 * (c * c - 1)) / a0, (c * c - c * sqrt(2) + 1) / a0]
        return b, a

    def get_hipass_coeffs(gain, cutoff, sr):
        c = get_c(cutoff, sr)
        a0 = c * c + c * sqrt(2) + 1
        b = [(gain * c * c) / a0, (-2 * gain * c * c) / a0, (gain * c * c) / a0]
        a = [1,                   (-2 * (c * c - 1)) / a0,  (c * c - c * sqrt(2) + 1) / a0]
        return b, a

    return twopass_coeffs([get_lopass_coeffs(gain, hi, sr), get_hipass_coeffs(gain, lo, sr)])

def get_notch_coeffs(gain, centre, sr, Q):
    A = 10 ** (gain / 40.0)
    w0 = 2 * pi * centre / sr
    cw0 = cos(w0)
    sw0 = sin(w0)
    alpha = sw0 / 2 * Q

    a0 = 1 + alpha / A

    b = [(1 + alpha * A) / a0, (-2 * cw0) / a0, (1 - alpha * A) / a0]
    a = [1, (-2 * cw0) / a0, (1 - alpha / A) / a0]

    return b, a

def main():
    edges = [30, 60, 120, 240, 480]
    corners = zip(edges[:-1], edges[1:])
    centres = [(a + b) / 2 for a, b in corners]

#c = [get_linkwitz_riley_coeffs(1, b, a, edges[-1] * 2) for b, a in corners]
    c = [get_notch_coeffs(np.random.random_sample() * -6, i, edges[-1] * 2, 1) for i in centres]
    c.append(series_coeffs(c))

    print c[-1]

    wh = [signal.freqz(b, a) for b, a in c]

    plt.subplot(111)
    plt.title("Frequency response")
    for w, h in wh:
        plt.semilogx(w, 20 * np.log10(np.abs(h)))
    plt.ylabel('Amplitude Response (dB)')
    plt.xlabel('Frequency (rad/sample)')
    plt.grid()

    plt.show()

if __name__ == "__main__":
    main()
