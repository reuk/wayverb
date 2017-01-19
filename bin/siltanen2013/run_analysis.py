#!/usr/local/bin/python

import analysis
import os.path


def main():
    root_dir = '/Users/reuben/development/waveguide/build/bin/siltanen2013'

    fnames = [os.path.join(root_dir, i) for i in [
        #'null.engine.wav',
        #'null.img_src.wav',
        #'null.waveguide.single_band.wav',
        #'null.waveguide.multiple_band.constant_spacing.wav',
        #'waveguide.44100.wav',
        #'raytracer.44100.wav',
        #'waveguide.5000.wav',
        #'raytracer.5000.wav',

        'a_0.05_null.img_src.aif',
        'a_0.05_null.waveguide.single_band.aif',

        'a_0.1_null.img_src.aif',
        'a_0.1_null.waveguide.single_band.aif',

        'a_0.2_null.img_src.aif',
        'a_0.2_null.waveguide.single_band.aif',
    ]]

    max_frequency = 120 

    analysis.modal_analysis(fnames, max_frequency, [5.56, 3.97, 2.81])

if __name__ == '__main__':
    main()
