@siltanen2013

calibration factor is

n = R / AX
    where
        R = distance at which the geometric sound source has intensity 1 W/m^2
        A = calibration coefficient (0.3405 for the rectilinear mesh)
        X = lambda fs / c
            where
                lambda = courant number (1 / sqrt(3) for rectilinear mesh)
                fs = sampling rate
                c = speed of sound
