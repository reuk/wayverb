#!/usr/local/bin/python

import analysis
import os.path


def main():
    root_dir = '../../build/utils/southern2013_2_cuboid'

    fnames = [os.path.join(root_dir, i) for i in [
        'raytracer_source_4_receiver_3_surface_0.wav',
        'raytracer_source_4_receiver_3_surface_1.wav',
        'raytracer_source_4_receiver_3_surface_2.wav',
        'waveguide_source_4_receiver_3_surface_0.wav',
        'waveguide_source_4_receiver_3_surface_1.wav',
        'waveguide_source_4_receiver_3_surface_2.wav',
    ]]

    max_frequency = 120
    room_dim = [5.56, 3.97, 2.81]

    analysis.modal_analysis(fnames, max_frequency, room_dim)

if __name__ == '__main__':
    main()
