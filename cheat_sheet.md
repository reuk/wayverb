waveguide unit conversion cheat sheet
=====================================

courant number = stability bound of the mesh
courant number = cT / X
    where
        c = speed of sound
        T = time step
        X = grid spacing
    2D: must be < 1/sqrt(2)
    3D: must be < 1/sqrt(3)
        D = mesh dimension

so:

    1 / sqrt(D) = cT / X

    X = cT sqrt(D)

    T = X / c sqrt(D)

    1 / T = c sqrt(D) / X

    c = X / T sqrt(D)
