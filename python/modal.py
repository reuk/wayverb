SPEED_OF_SOUND = 340


def mode_frequency(orders, dimensions):
    return SPEED_OF_SOUND / 2 * \
        pow(sum([pow(orders[i] / dimensions[i], 2) for i in range(3)]), 0.5)


def main():
    orders = [
        [0, 0, 0],
        [1, 0, 0],
        [0, 1, 0],
        [0, 0, 1],
        [1, 1, 0],
        [1, 0, 1],
        [0, 1, 1],
        [1, 1, 1],
    ]
    for order in orders:
        print mode_frequency(order, [5.56, 3.97, 2.81])

if __name__ == "__main__":
    main()
