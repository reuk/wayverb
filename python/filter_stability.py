import numpy as np
from boundary_modelling import get_notch_coeffs, series_coeffs
from collections import namedtuple

Surface = namedtuple('Surface', ['specular', 'diffuse'])

def a2db(a):
    return 20 * np.log10(a)

def to_filter_coefficients(surface, sr):
    num_descriptors = 3
    edges = [40, 175, 350, 700, 1400, 2800, 5600, 11200, 20000]
    coeffs = []
    for i in range(num_descriptors):
        gain = a2db((surface.specular[i] + surface.diffuse[i]) * 0.5)
        centre = (edges[i] + edges[i + 1]) * 0.5
        coeffs.append(get_notch_coeffs(gain, centre, sr, 1.414))
    return coeffs

def is_stable_roots(polynomial):
    return np.all(np.abs(np.roots(polynomial)) < 1)

def is_stable_recursive(a):
    if len(a) == 0:
        return True

    rci = a[-1]
    if np.abs(rci) >= 1:
        return False

    next_size = len(a) - 1
    next_array = []
    for i in range(next_size):
        next_array.append((a[i] - rci * a[next_size - i]) / (1 - rci * rci))

    return is_stable_recursive(next_array)

def random_coeffs():
    return [1] + np.random.rand(20) - 0.5

def random_coeffs_from_surface():
    surface_desc = np.random.rand(8)
    surface = Surface(surface_desc, surface_desc)
    coeffs = to_filter_coefficients(surface, 44100)
    for c in coeffs:
        polynomial = c[1]
        stable_roots = is_stable_roots(polynomial)
        stable_recursive = is_stable_recursive(polynomial)
        if stable_roots != stable_recursive:
            raise RuntimeError("results don't match for polynomial ", polynomial)
    return series_coeffs(coeffs)[1]

def main():
    for i in range(2000):
        polynomial = random_coeffs_from_surface()
        stable_roots = is_stable_roots(polynomial)
        stable_recursive = is_stable_recursive(polynomial)
        if stable_roots != stable_recursive:
            print "failing polynomial: ", polynomial
            return False
    return True

def test():
    print is_stable_roots(to_filter_coefficients(Surface(
            [0.4, 0.3, 0.5, 0.8, 0.9, 1, 1, 1],
            [0.4, 0.3, 0.5, 0.8, 0.9, 1, 1, 1]
        ), 44100)[1])

if __name__ == "__main__":
    print main()
