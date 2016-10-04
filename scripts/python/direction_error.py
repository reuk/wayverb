from math import e, pi
import numpy as np

import matplotlib
render = False
if render:
    matplotlib.use('pgf')
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib import colors, ticker, cm
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import operator
import scipy.optimize

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

def get_cubic_vectors():
    return [np.array([-1,  0,  0]), np.array([ 1,  0,  0]),
            np.array([ 0, -1,  0]), np.array([ 0,  1,  0]),
            np.array([ 0,  0, -1]), np.array([ 0,  0,  1])]

# direction error analysis from @hacihabiboglu

# p(x) = pressure field in spatial(?) domain
# P(w) = pressure field in frequency domain

def get_U(v):
    U = np.vstack(v)
    return U

def eq_21(u, w, pressure):
    return pressure * (pow(e, -1j * np.dot(u, w)) - 1)

def eq_22(w, pressure, v):
    return np.array([eq_21(i, w, pressure) for i in v])

def eq_23(w, pressure, v):
    return np.dot(np.linalg.pinv(get_U(v)), eq_22(w, pressure, v))

def hermitian_angle(a, b):
    prod = np.dot(a, b)
    mag_a = np.sqrt(np.dot(a, np.conj(a)))
    mag_b = np.sqrt(np.dot(b, np.conj(b)))
    return np.degrees(np.arccos((prod / (mag_a * mag_b)).real))

def direction_difference(arr, v):
    pressure = 1
    def get_term_1():
        return eq_23(arr, pressure, v)
    def get_term_2():
        return 1j * arr * pressure
    return hermitian_angle(get_term_1(), get_term_2())

def sphere_vector(azimuth, elevation, r):
    x = r * np.cos(elevation) * np.cos(azimuth)
    y = r * np.cos(elevation) * np.sin(azimuth)
    z = r * np.sin(elevation)
    return np.array([x, y, z])

# azimuth: around, -pi to pi, elevation: up/down, -pi/2 to pi/2
def direction_difference_for_angles(angles, r, v):
    return -direction_difference(sphere_vector(*angles, r=r), v)

# monte carlo bandwidth estimation
def random_three_vector():
    phi = np.random.uniform(0, pi * 2)
    costheta = np.random.uniform(-1, 1)
    theta = np.arccos(costheta)
    return sphere_vector(phi, theta)

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
    This program replicates the direction error analysis from the paper
    Simulation of Directional Microphones in Digital Waveguide Mesh-Based
    Models of Room Acoustics
    """

    # let's find the maximum value here
    # trying to optimize for direction_difference (?)
    x0 = [0.2, 0.2]
    rad = pi/4
    v = get_base_vectors(True)
    #v = get_cubic_vectors()
    res = scipy.optimize.minimize(direction_difference_for_angles, x0, (rad, v,))
    print "rad:", rad, "maximum error:", -res.fun

    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'font.monospace': ['Input Mono Condensed'],
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    func = direction_difference
    vfunc = np.vectorize(lambda x, y, z: func(np.array([x, y, z]), v))

    plt.figure(figsize=(12, 3), dpi=100)

    for i, max_val in zip(range(3), [pi / 4, pi / 8, pi / 16]):
        phi, theta = np.mgrid[-pi:pi:60j, -pi/2:pi/2:30j]
        XX, YY, ZZ = sphere_vector(phi, theta, max_val)
        zz = vfunc(XX, YY, ZZ)
        zzmin, zzmax = zz.min(), zz.max()

        my_cm = cm.jet
        m = cm.ScalarMappable(cmap=my_cm)
        m.set_array(zz)
        zz = (zz - zzmin) / (zzmax - zzmin)

        ax = plt.subplot(131 + i, projection='3d')
        ax.set_aspect("equal")
        surf = ax.plot_surface(
                XX, YY, ZZ, rstride=1, cstride=1, facecolors=my_cm(zz), linewidth=0)
        plt.colorbar(m)
        bounds = 0.8
        ax.auto_scale_xyz([-bounds, bounds], [-bounds, bounds], [-bounds, bounds])
        plt.axis("off")

    plt.show()
    if render:
        plt.savefig("direction_error_in_tetrahedral_mesh.pdf", bbox_inches="tight")

if __name__ == "__main__":
    main()
