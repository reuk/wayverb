import numpy as np

COURANT = 1 / np.sqrt(3.0)
COURANT_SQ = 1 / 3.0

def eq_1d(inner_pressure, pressure_a, pressure_b,
          pressure_c, pressure_d, prev_pressure, b0, a0, gn):
    scale = COURANT * a0 / b0
    term_1 = (COURANT_SQ * (2 * inner_pressure + pressure_a + pressure_b + pressure_c + pressure_d))
    term_2 = (COURANT_SQ * gn / b0)
    term_3 = ((scale - 1) * prev_pressure)
    return  (term_1 + term_2 + term_3) / (scale + 1)


def main():
    print eq_1d(1, 0, 0, 0, 0, 0, 1, 0, 0)

if __name__ == "__main__":
    main()
