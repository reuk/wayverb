import numpy as np
import scipy.signal as signal
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from boundary_modelling import (get_peak_coeffs,
                                series_coeffs,
                                db2a,
                                a2db,
                                impedance_filter)
from collections import namedtuple


def zplane(b, a):
    z, p, k = signal.tf2zpk(b, a)

    # plt.figure()
    plt.title("Pole-zero placement of EQ Filters")
    plt.plot(z.real, z.imag, 'ko', fillstyle='none', ms=10)
    plt.plot(p.real, p.imag, 'kx', fillstyle='none', ms=10)

    unit_circle = patches.Circle(
        (0, 0),
        radius=1,
        fill=False,
        color='black',
        ls='solid',
        alpha=0.9)
    plt.gca().add_patch(unit_circle)
    plt.axvline(0, color='0.7')
    plt.axhline(0, color='0.7')
    plt.grid()
    plt.ylim([-1, 1])
    plt.xlim([-1, 1])
    plt.gca().set_aspect('equal', adjustable='box')

    plt.ylabel('Imaginary')
    plt.xlabel('Real')

Surface = namedtuple('Surface', ['specular', 'diffuse'])


def to_filter_coefficients(surface, sr):
    num_descriptors = 3
    edges = [40, 175, 350, 700, 1400, 2800, 5600, 11200, 20000]
    coeffs = []
    for i in range(num_descriptors):
        gain = a2db((surface.specular[i] + surface.diffuse[i]) * 0.5)
        centre = (edges[i] + edges[i + 1]) * 0.5
        coeffs.append(get_peak_coeffs(gain, centre, sr, 1.414))
    return coeffs


def is_stable_roots(polynomial):
    return np.all(np.abs(np.roots(polynomial)) < 1)


def is_stable_recursive(a):
    if len(a) == 1:
        return True

    rci = a[-1]
    print rci
    if np.abs(rci) >= 1:
        return False

    next_size = len(a) - 1
    next_array = []
    for i in range(next_size):
        next_array.append((a[i] - rci * a[next_size - i]) / (1 - rci * rci))

    return is_stable_recursive(next_array)


def is_stable_jury(polynomial):
    vvd = []
    vvd.append(polynomial)
    vvd.append(polynomial[::-1])

    i = 2
    while True:
        v = []
        mult = vvd[i - 1][len(vvd[i - 2]) - 1] / vvd[i - 2][0]
        for j in range(len(vvd[i - 2]) - 1):
            v.append(vvd[i - 2][j] - vvd[i - 1][j] * mult)
        vvd.append(v)
        vvd.append(v[::-1])
        if len(v) == 1:
            break
        i += 2

    for j in range(0, len(vvd), 2):
        if vvd[j][0] <= 0:
            break

    return i == len(vvd)


def all_equal(x):
    return x.count(x[0]) == len(x)


def is_stable(polynomial):
    print polynomial
    stable_roots = is_stable_roots(polynomial)
    stable_recursive = is_stable_recursive(polynomial)
    stable_jury = is_stable_jury(polynomial)
    results = [
            stable_roots,
            stable_recursive,
            # stable_jury,
    ]
    if not all_equal(results):
        raise RuntimeError("results don't match for polynomial ",
                           polynomial,
                           " with results: ",
                           results)
    return stable_roots


def random_coeffs():
    return [1] + np.random.rand(20) - 0.5


def check_surface_filters(surface_desc, check):
    coeffs = to_filter_coefficients(
        Surface(
            surface_desc,
            surface_desc),
        44100)
    if check:
        for c in coeffs:
            is_stable(c[1])
    coeffs = series_coeffs(coeffs)
    if check:
        is_stable(coeffs[1])
    coeffs = impedance_filter(coeffs)
    if check:
        is_stable(coeffs[1])
    return coeffs


def random_coeffs_from_surface():
    surface = np.random.rand(8)
    return check_surface_filters(surface, False)


def do_graph(coeffs, yes):
    try:
        return is_stable(coeffs)
    except:
        if yes:
            zplane(coeffs, coeffs)
            plt.show()
        return False


def main():
    for i in range(100000):
        if not do_graph(random_coeffs_from_surface()[1], True):
            return False
    return True


def test():
    stable = []
    for i in [
        [0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9],
        [0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1],
        [0.4, 0.3, 0.5, 0.8, 0.9, 0.9, 0.9, 0.9],
    ]:
        coeffs = check_surface_filters(i, False)
        stable.append(do_graph(coeffs[1], True))

    return stable

if __name__ == "__main__":
    print test()
    # print main()
