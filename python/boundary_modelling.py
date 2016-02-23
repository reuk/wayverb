from math import pi, sin, cos, tan, sqrt
from recordclass import recordclass
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

BiquadMemory = recordclass('BiquadMemory', ['z1', 'z2'])
BiquadCoefficients = recordclass('BiquadCoefficients', ['b0', 'b1', 'b2', 'a1', 'a2'])

def biquad_step(i, bm, bc):
    out   = i * bc.b0               + bm.z1
    bm.z1 = i * bc.b1 - bc.a1 * out + bm.z2
    bm.z2 = i * bc.b2 - bc.a2 * out
    return out

def biquad_cascade(i, bm, bc):
    for m, c in zip(bm, bc):
        i = biquad_step(i, m, c)
    return i

def impedance_filter(c):
    num = c[0]
    den = c[1]

    summed = [a + b for a, b in zip(den, num)]
    subbed = [a - b for a, b in zip(den, num)]

    norm = 1 / subbed[0]
    summed = [i * norm for i in summed]
    subbed = [i * norm for i in subbed]

    return [summed, subbed]

def eighth_order_step(i, m, c):
    out  = i * c[0][0]                 + m[0]
    m[0] = i * c[0][1] - c[1][1] * out + m[1]
    m[1] = i * c[0][2] - c[1][2] * out + m[2]
    m[2] = i * c[0][3] - c[1][3] * out + m[3]
    m[3] = i * c[0][4] - c[1][4] * out + m[4]
    m[4] = i * c[0][5] - c[1][5] * out + m[5]
    m[5] = i * c[0][6] - c[1][6] * out + m[6]
    m[6] = i * c[0][7] - c[1][7] * out + m[7]
    m[7] = i * c[0][8] - c[1][8] * out
    return out

def main():
    edges = [30, 60, 120, 240]
    corners = zip(edges[:-1], edges[1:])
    centres = [(a + b) / 2 for a, b in corners]

#c = [get_linkwitz_riley_coeffs(1, b, a, edges[-1] * 2) for b, a in corners]
    sr = 2000
    c = [get_notch_coeffs(-24, i, sr, 1) for i in centres]

    bm = [BiquadMemory(0, 0) for _ in c]
    bc = [BiquadCoefficients(b0, b1, b2, a1, a2) for [b0, b1, b2], [a0, a1, a2] in c]

    c.append(series_coeffs(c))

    c.append(impedance_filter(c[-1]))

    wh = [signal.freqz(b, a) for b, a in c]

    sig = [1, 0, 0, 0, 0, 0, 0]

    plt.subplot(111)
    plt.title("Frequency response - reflection filter")
    for w, h in wh:
        plt.semilogx(w, 20 * np.log10(np.abs(h)))
    plt.ylabel('Amplitude Response (dB)')
    plt.xlabel('Frequency (rad/sample)')
    plt.grid()

    plt.show()

if __name__ == "__main__":
    main()
