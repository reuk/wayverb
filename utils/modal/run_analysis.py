#!/usr/local/bin/python

import analysis
import os.path


def main():
    root_dir = '../../build/utils/siltanen2013'

    fnames = [os.path.join(root_dir, i) for i in [
        'raytracer.windowed.wav',
        'waveguide.windowed.wav',
        #'raw_img_src_attenuated_0.wav',
        #'img_src_and_waveguide_source_0_receiver_0_surface_0.wav',
        #'image_source_source_0_receiver_0_surface_0.wav',
        #'raytracer_source_4_receiver_3_surface_0.wav',
        #'raytracer_source_4_receiver_3_surface_1.wav',
        #'raytracer_source_4_receiver_3_surface_2.wav',
        #'waveguide_source_4_receiver_3_surface_0.wav',
        #'waveguide_source_4_receiver_3_surface_1.wav',
        #'waveguide_source_4_receiver_3_surface_2.wav',
    ]]

    max_frequency = 120
    room_dim = [5.56, 3.97, 2.81]

    analysis.modal_analysis(fnames, max_frequency, room_dim)

if __name__ == '__main__':
    main()
