import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
from numpy import sqrt
import operator

BASIC_CUBE = [(0, 0, 0),            # 0
              (0.5, 0, 0.5),        # 1
              (0.25, 0.25, 0.25),   # 2
              (0.75, 0.25, 0.75),   # 3
              (0, 0.5, 0.5),        # 4
              (0.5, 0.5, 0),        # 5
              (0.25, 0.75, 0.75),   # 6
              (0.75, 0.75, 0.25)]   # 7

def mul((x, y, z), d):
    return (x * d, y * d, z * d)

def node_cube(spacing):
    return map(lambda i: mul(i, spacing), BASIC_CUBE)

def get_mesh((x, y, z), spacing):
    c = []
    for i in range(x):
        for j in range(y):
            for k in range(z):
                nodes = node_cube(spacing)
                nodes = map(lambda (a, b, c): (a + i * spacing, b + j * spacing, c + k * spacing), nodes)
                c += nodes
    return c

class Locator:
    def __init__(self, pos, mod_ind):
        self.pos = pos
        self.mod_ind = mod_ind

class WaveguideMesh:
    def __init__(self, dim, spacing):
        self.mesh = get_mesh(dim, spacing)
        self.dim = dim

    def get_index(self, locator):
        i, j, k = self.dim
        x, y, z = locator.pos
        l = len(BASIC_CUBE)
        return locator.mod_ind + x * l + y * i * l + z * i * j * l

    def get_neighbor_offsets(self, mod_ind):
        ret =  [[((0, 0, 0), 2), ((-1, 0, -1), 3), ((0, -1, -1), 6), ((-1, -1, 0), 7)],
                [((0, 0, 0), 2), ((0, 0, 0), 3), ((0, -1, 0), 6), ((0, -1, 0), 7)],
                [((0, 0, 0), 0), ((0, 0, 0), 1), ((0, 0, 0), 4), ((0, 0, 0), 5)],
                [((0, 0, 0), 2), ((1, 0, 0), 4), ((0, 0, 1), 5), ((1, 0, 1), 0)],
                [((0, 0, 0), 2), ((-1, 0, 0), 3), ((0, 0, 0), 6), ((0, 0, -1), 7)],
                [((0, 0, 0), 2), ((0, 0, -1), 3), ((0, 0, 0), 7), ((0, 0, -1), 6)],
                [((0, 1, 1), 0), ((0, 1, 0), 1), ((0, 0, 0), 4), ((0, 0, 1), 5)],
                [((1, 1, 0), 0), ((0, 1, 0), 1), ((0, 0, 1), 4), ((0, 0, 0), 5)]][mod_ind]
        return [self.get_index(Locator(*i)) for i in ret]

    def get_absolute_neighbors(self, index):
        mod_ind = index % len(BASIC_CUBE)
        off = index - mod_ind
        return map(lambda i: i + off, self.get_neighbor_offsets(mod_ind))

def concat(l):
    return reduce(operator.add, l)

def main():
    waveguide = WaveguideMesh((5, 5, 5), 1)
    x, y, z = map(lambda i: np.array(i), zip(*waveguide.mesh))

    fig = plt.figure()

    for plot in range(8):
        ax = fig.add_subplot(241 + plot, projection='3d', aspect='equal')

        pos = 400 + plot
        n = waveguide.get_absolute_neighbors(pos)
        p = [waveguide.mesh[i] for i in n]
        p += [waveguide.mesh[pos]]
        ax.scatter(*zip(*p))

        max_range = np.array([x.max() - x.min(), y.max() - y.min(), z.max() - z.min()]).max() / 2.0

        mean_x = x.mean()
        mean_y = y.mean()
        mean_z = z.mean()

        ax.set_xlim(mean_x - max_range, mean_x + max_range)
        ax.set_ylim(mean_y - max_range, mean_y + max_range)
        ax.set_zlim(mean_z - max_range, mean_z + max_range)

        ax.set_xlabel("x")
        ax.set_ylabel("y")
        ax.set_zlabel("z")
    plt.show()

if __name__ == "__main__":
    main()
