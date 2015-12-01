import numpy as np
import matplotlib.pyplot as plt
from string import split
import math

def main():
    bands = 7
    for i in range(bands):
        energies = []
        max_vals = []
        with open("cardioid.energies.txt") as f:
            for line in f:
                energy = float(split(line)[5 + i * 6])
                max_val = float(split(line)[7 + i * 6])
                energies.append(energy)
                max_vals.append(max_val)

        m = max(energies)
        energies = [e / m for e in energies]

        theta = np.linspace(0, 2 * np.pi, len(energies), False)

        shape = 0.5
        desired = [(1 - shape) + shape * np.cos(i) for i in theta]

        errors = [np.abs((b - a) / b) for a, b in zip(energies, desired)]
        errors = [val for val in errors if not (math.isnan(val) or math.isinf(val))]
        print "max error:", max(errors)
        print "mean error:", sum(errors) / len(errors)

        ax = plt.subplot(241 + i, projection='polar')
        ax.plot(theta, energies, color='r', linewidth = 3)
        ax.plot(theta, desired, color='g', linewidth = 3)
        #ax.plot(theta, max_vals, color='b', linewidth = 3)
        #ax.set_rmax(max(max(energies), max(max_vals)))
        ax.grid(True)

    plt.show()

if __name__ == "__main__":
    main()
