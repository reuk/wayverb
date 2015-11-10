from math import e, pi
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import colors, ticker, cm
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import operator

def random_three_vector():
    phi = np.random.uniform(0, pi * 2)
    costheta = np.random.uniform(-1, 1)
    theta = np.arccos(costheta)
    x = np.sin(theta) * np.cos(phi)
    y = np.sin(theta) * np.sin(phi)
    z = np.cos(theta)
    return np.array([x, y, z])

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

def get_b(arr):
    summed = sum([pow(e, 1j * np.dot(arr, i)) for i in get_vectors()])
    return 1.0 - 0.25 * summed.real

def get_ang_g(arr):
    b = get_b(arr)
    return 0.5 * np.arctan(np.sqrt(4 - b * b) / abs(b))

def get_speed(arr):
    """
    The diagrams in the paper appear to be continuous outside of the range
    -1.5, 1.5.
    However, this function has a strange discontinuity at a radius of 1.4
    """

    c = np.sqrt(1.0 / 3.0)
    norm = np.linalg.norm(arr)

    # this analysis is only valid for frequencies below pi / 2
    # (spectrum is mirrored above this limit)

    # simulated frequency is equal to magnitude of wave vector (arr)
    if norm < pi / 2:
        return get_ang_g(arr) / (norm * c)
    else:
        return None

def get_max_valid_frequency(accuracy, starting_freq, increments, samples):
    last = starting_freq + increments
    ret = starting_freq
    while True:
        sample_points = [random_three_vector() * last for i in range(samples)]
        sampled = [get_speed(i) for i in sample_points]
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

    min_accuracy = 0.95
    max_val = get_max_valid_frequency(min_accuracy, 0.1, 0.001, 20)
    print "maximum radius (frequency): ", max_val / (pi / 2)
    phi, theta = np.mgrid[0:pi:50j, 0:2*pi:50j]
    XX = max_val * np.sin(phi) * np.cos(theta)
    YY = max_val * np.sin(phi) * np.sin(theta)
    ZZ = max_val * np.cos(phi)
    zz = np.vectorize(lambda x, y, z: get_speed(np.array([x, y, z])))(XX, YY, ZZ)
    zzmin, zzmax = zz.min(), zz.max()
    zz = (zz - zzmin) / (zzmax - zzmin)

    fig = plt.figure()

    bounds = pi / 2
    N = 100
    x = np.linspace(-bounds, bounds, N)
    y = np.linspace(-bounds, bounds, N)
    X, Y = np.meshgrid(x, y)
    vfunc = np.vectorize(lambda x, y, z: get_speed(np.array([x, y, z])))
    Z = np.zeros(X.shape)
    depth = np.linspace(0.9, 1, 11)

    ### plot 1
    ax = fig.add_subplot(231 + 0)
    z = vfunc(Z, X, Y)
    plt.contourf(X, Y, z, depth)
    cbar = plt.colorbar()

    ### plot 2
    ax = fig.add_subplot(231 + 1)
    z = vfunc(X, Z, Y)
    plt.contourf(X, Y, z, depth)
    cbar = plt.colorbar()

    ### plot 3
    ax = fig.add_subplot(231 + 2)
    z = vfunc(X, Y, Z)
    plt.contourf(X, Y, z, depth)
    cbar = plt.colorbar()

    ax = fig.add_subplot(212, projection='3d')
    ax.plot_surface(
            XX, YY, ZZ, rstride=1, cstride=1, facecolors=cm.jet(zz))

    plt.show()

if __name__ == "__main__":
    main()
