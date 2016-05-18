waveguide unit conversion cheat sheet
=====================================

courant number = stability bound of the mesh
courant number = cT / X
    where
        c = speed of sound
        T = time step
        X = grid spacing
        D = mesh dimension
    2D: must be < 1/sqrt(2)
    3D: must be < 1/sqrt(3)

so:

    1 / sqrt(D) = cT / X

so:
    X = cT sqrt(D)

    T = X / c sqrt(D)

    1 / T = c sqrt(D) / X

    c = X / T sqrt(D)
