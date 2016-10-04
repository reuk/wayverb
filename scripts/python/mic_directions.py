from math import e, pi
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import colors, ticker, cm
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import operator

def pressure(pos, time):
    """lookup in waveguide mesh"""
    return 0

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

def get_U(flip):
    v = get_base_vectors(flip)
    U = np.vstack(v)
    print U
    return U

def directional_derivative(pos, u, time, interjunction_distance):
    return (pressure(u, time) - pressure(pos, time)) / interjunction_distance

def all_derivatives(pos, time, interjunction_distance, flip):
    return [directional_derivative(pos, pos + u, time, interjunction_distance) for u in get_base_vectors(flip)]

def approximated_pressure_gradient(pos, time, interjunction_distance, flip):
    return np.dot(np.linalg.pinv(get_U(flip)), all_derivatives(pos, time, interjunction_distance, flip))

def time_derivative_of_velocity(pos, time, interjunction_distance, flip, ambient_density):
    return -approximated_pressure_gradient(pos, time, interjunction_distance, flip) / ambient_density

def velocity(pos, time, interjunction_distance, flip, ambient_density):
    if time <= 0:
        return 0
    else:
        return (velocity(pos, time - 1, interjunction_distance, flip, ambient_density) +
            time_derivative_of_velocity(pos, time - 1, interjunction_distance, flip, ambient_density))

def intensity(pos, time, interjunction_distance, flip, ambient_density):
    return pressure(pos, time) * velocity(pos, time, interjunction_distance, flip, ambient_density)

def main():
    print intensity(np.array([0, 0, 0]), 1, 1, True, 1)

if __name__ == "__main__":
    main()
