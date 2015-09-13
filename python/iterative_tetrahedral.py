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

def add(a, b):
    return map(lambda (x, y): x + y, zip(a, b))

def node_cube(spacing):
    return map(lambda i: mul(i, spacing), BASIC_CUBE)

def get_mesh((x, y, z), spacing):
    c = []
    for i in range(x):
        xo = i * spacing
        for j in range(y):
            yo = j * spacing
            for k in range(z):
                zo = k * spacing
                nodes = node_cube(spacing)
                nodes = map(lambda i: add(i, (xo, yo, zo)), nodes)
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
        self.offsets = self.get_neighbor_offset_table()

    def get_index(self, locator):
        i, j, k = self.dim
        x, y, z = locator.pos
        l = len(BASIC_CUBE)
        return locator.mod_ind + x * l + y * i * l + z * i * j * l

    def get_locator(self, index):
        i, j, k = self.dim
        mod_ind = index % len(BASIC_CUBE)
        index -= mod_ind
        index /= len(BASIC_CUBE)
        x = index % i
        index -= x
        index /= i
        y = index % j
        index -= y
        index /= j
        z = index % k
        index -= z
        index /= k
        return Locator((x, y, z), mod_ind)

    def get_neighbor_offset_table(self):
        ret = [[((0, 0, 0), 2), ((-1, 0, -1), 3), ((-1, -1, 0), 6), ((0, -1, -1), 7)],
               [((0, 0, 0), 2), ((0, 0, 0), 3), ((0, -1, 0), 6), ((0, -1, 0), 7)],
               [((0, 0, 0), 0), ((0, 0, 0), 1), ((0, 0, 0), 4), ((0, 0, 0), 5)],
               [((1, 0, 1), 0), ((0, 0, 0), 1), ((0, 0, 1), 4), ((1, 0, 0), 5)],
               [((0, 0, 0), 2), ((0, 0, -1), 3), ((0, 0, 0), 6), ((0, 0, -1), 7)],
               [((0, 0, 0), 2), ((-1, 0, 0), 3), ((-1, 0, 0), 6), ((0, 0, 0), 7)],
               [((1, 1, 0), 0), ((0, 1, 0), 1), ((0, 0, 0), 4), ((1, 0, 0), 5)],
               [((0, 1, 1), 0), ((0, 1, 0), 1), ((0, 0, 1), 4), ((0, 0, 0), 5)]]
        return map(lambda j: map(lambda i: Locator(*i), j), ret)

    def get_neighbor_offsets(self, mod_ind):
        return self.offsets[mod_ind]

    def locator_filter(self, c, relative):
        x, y, z = self.dim
        rlx, rly, rlz = add(c.pos, relative.pos)
        return 0 <= rlx < x and 0 <= rly < y and 0 <= rlz < z

    def get_absolute_neighbors(self, index):
        locator = self.get_locator(index)
        x, y, z = locator.pos
        mod_ind = locator.mod_ind

        relative = self.get_neighbor_offsets(mod_ind)
        relative = filter(lambda x: self.locator_filter(locator, x), relative)

        n = map(lambda i: self.get_index(i) + self.get_index(Locator((x, y, z), 0)), relative)
        return n

def concat(l):
    return reduce(operator.add, l)

def main():
    waveguide = WaveguideMesh((2, 2, 2), 1)
    x, y, z = map(lambda i: np.array(i), zip(*waveguide.mesh))
    max_range = np.array([x.max() - x.min(), y.max() - y.min(), z.max() - z.min()]).max() / 2.0
    mean_x = x.mean()
    mean_y = y.mean()
    mean_z = z.mean()

    fig = plt.figure()

    for plot in range(8):
        ax = fig.add_subplot(331 + plot, projection='3d', aspect='equal')

        pos = waveguide.get_index(Locator((0, 0, 0), plot))
        n = waveguide.get_absolute_neighbors(pos)
        p = []
        p += [waveguide.mesh[i] for i in n]
        p += [waveguide.mesh[pos]]
        print plot, p

        ax.scatter(*zip(*p))

        ax.set_xlim(mean_x - max_range, mean_x + max_range)
        ax.set_ylim(mean_y - max_range, mean_y + max_range)
        ax.set_zlim(mean_z - max_range, mean_z + max_range)

        ax.set_xlabel("x")
        ax.set_ylabel("y")
        ax.set_zlabel("z")

    ax = fig.add_subplot(339, projection='3d', aspect='equal')
    ax.scatter(*zip(*waveguide.mesh))

    ax.set_xlim(mean_x - max_range, mean_x + max_range)
    ax.set_ylim(mean_y - max_range, mean_y + max_range)
    ax.set_zlim(mean_z - max_range, mean_z + max_range)

    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.set_zlabel("z")

    plt.show()

if __name__ == "__main__":
    main()
