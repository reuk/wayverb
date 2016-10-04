from scipy.signal import dlsim
from numpy import zeros
import matplotlib.pyplot as plt

from filter_stability import check_surface_filters


def main():
    dirac = zeros((100, 1))
    dirac[0] = 1

    for i in [
        [0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9],
    ]:
        reflectance = check_surface_filters(i, False)
        plt.figure()
        plt.plot(*dlsim((reflectance[0], reflectance[1], 1), dirac))

        plt.show()

if __name__ == "__main__":
    main()
