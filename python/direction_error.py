from math import e, pi
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import colors, ticker, cm
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import operator

def get_base_vectors(flip):
    ret = [
            np.array([0.0,                 2.0 * np.sqrt(2.0) / 3.0, 1.0 / 3.0]),
            np.array([ np.sqrt(2.0 / 3.0), -np.sqrt(2.0) / 3.0,      1.0 / 3.0]),
            np.array([0.0,                 0.0,                      -1.0]),
            np.array([-np.sqrt(2.0 / 3.0), -np.sqrt(2.0) / 3.0,      1.0 / 3.0]),
        ]

    if flip:
        ret = [np.array([1, -1, -1]) * i for i in ret]

    return ret

def get_vectors():
    ret = [i + j for i in get_base_vectors(False) for j in get_base_vectors(True)]
    ret = filter(lambda x: np.any(x != np.array([0, 0, 0])), ret)
    return ret

# direction error analysis from @hacihabiboglu

# p(x) = pressure field in spatial(?) domain
# P(w) = pressure field in frequency domain

def get_U():
    v = get_base_vectors(True)
    U = np.vstack(v)
    return U

def eq_21(u, w, pressure):
    return pressure * (pow(e, -1j * np.dot(u, w)) - 1)

def eq_22(w, pressure):
    return np.array([eq_21(i, w, pressure) for i in get_base_vectors(True)])

def eq_23(w, pressure):
    return np.dot(np.linalg.pinv(get_U()), eq_22(w, pressure))

def hermitian_angle(a, b):
    prod = np.dot(a, np.conj(b))
    mag_a = np.sqrt(np.dot(a, np.conj(a)))
    mag_b = np.sqrt(np.dot(b, np.conj(b)))
    return (prod / (mag_a * mag_b)).real

def direction_difference(arr):
    pressure = 1
    def get_term_1():
        return eq_23(arr, pressure)
    def get_term_2():
        return 1j * arr * pressure
    return hermitian_angle(get_term_1(), get_term_2())

# monte carlo bandwidth estimation
def random_three_vector():
    phi = np.random.uniform(0, pi * 2)
    costheta = np.random.uniform(-1, 1)
    theta = np.arccos(costheta)
    x = np.sin(theta) * np.cos(phi)
    y = np.sin(theta) * np.sin(phi)
    z = np.cos(theta)
    return np.array([x, y, z])

def get_max_valid_frequency(func, accuracy, starting_freq, increments, samples):
    last = starting_freq + increments
    ret = starting_freq
    while True:
        sample_points = [random_three_vector() * last for i in range(samples)]
        sampled = [func(i) for i in sample_points]
        if not all(map(lambda x: x > accuracy, sampled)):
            return ret
        else:
            ret = last
            last += increments

def main():
    """
    This program duplicates the tetrahedral dispersion diagrams from the paper
    'The Tetrahedral Digital Waveguide Mesh' buy Duyne and Smith.

    I wrote it to try to understand how to do dispersion analysis - the
    analysis here is of the difference of the actual wavefront speed to the
    ideal speed.
    """

    func = direction_difference
    vfunc = np.vectorize(lambda x, y, z: func(np.array([x, y, z])))

    max_val = np.pi / 4
    phi, theta = np.mgrid[0:pi:50j, 0:2*pi:50j]
    XX = max_val * np.sin(phi) * np.cos(theta)
    YY = max_val * np.sin(phi) * np.sin(theta)
    ZZ = max_val * np.cos(phi)
    zz = vfunc(XX, YY, ZZ)
    zzmin, zzmax = zz.min(), zz.max()
    print "dispersion error range:", zzmin, "to", zzmax
    zz = (zz - zzmin) / (zzmax - zzmin)

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.plot_surface(
            XX, YY, ZZ, rstride=1, cstride=1, facecolors=cm.jet(zz))
    plt.show()

if __name__ == "__main__":
    main()
