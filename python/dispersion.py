from math import e
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

def get_b(x, y, z):
    summed = sum([pow(e, 1j * np.dot(np.array([x, y, z]), i)) for i in get_vectors()])
    return 1.0 - 0.25 * summed

def get_g_sqr(x, y, z):
    b = get_b(x, y, z)
    return -(b / 2.0) + 1j * (np.sqrt(4 - b * b) / 2)

def get_ang_g(x, y, z):
    b = get_b(x, y, z)
    return 0.5 * max(np.arctan(np.sqrt(4 - b * b) / b), np.arctan(-np.sqrt(4 - b * b) / b))

def get_speed(x, y, z):
    c = np.sqrt(1.0 / 3.0)
    norm = np.linalg.norm(np.array([x, y, z]))
    return get_ang_g(x, y, z) / (norm * c)

def main():
    """
    This program duplicates the tetrahedral dispersion diagrams from the paper
    'The Tetrahedral Digital Waveguide Mesh' buy Duyne and Smith.

    I wrote it to try to understand how to do dispersion analysis - the
    analysis here is of the difference of the actual wavefront speed to the
    ideal speed.
    """

    fig = plt.figure()

    bounds = 1.5
    N = 100
    x = np.linspace(-bounds, bounds, N)
    y = np.linspace(-bounds, bounds, N)
    X, Y = np.meshgrid(x, y)
    Z = np.zeros(X.shape)

    ### plot 1
    ax = fig.add_subplot(131 + 0)
    z = np.vectorize(get_speed)(Z, X, Y)
    #plt.contourf(X, Y, z, np.linspace(0.9, 1, 11))
    plt.contour(X, Y, z, np.linspace(0.9, 1, 11))
    cbar = plt.colorbar()

    ### plot 2
    ax = fig.add_subplot(131 + 1)
    z = np.vectorize(get_speed)(X, Z, Y)
    #plt.contourf(X, Y, z, np.linspace(0.9, 1, 11))
    plt.contour(X, Y, z, np.linspace(0.9, 1, 11))
    cbar = plt.colorbar()

    ### plot 1
    ax = fig.add_subplot(131 + 2)
    z = np.vectorize(get_speed)(X, Y, Z)
    #plt.contourf(X, Y, z, np.linspace(0.9, 1, 11))
    plt.contour(X, Y, z, np.linspace(0.9, 1, 11))
    cbar = plt.colorbar()

    plt.show()

if __name__ == "__main__":
    main()
