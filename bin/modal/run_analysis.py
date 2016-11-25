#!/usr/local/bin/python

import analysis
import os.path


def main():
    #root_dir = '/Users/reuben/development/waveguide/build/bin/siltanen2013'
    root_dir = '/Users/reuben/development/waveguide/xc/bin/siltanen2013/Debug'

    fnames = [os.path.join(root_dir, i) for i in [
        #'null.engine.wav',
        #'null.img_src.wav',
        #'null.waveguide.single_band.wav',
        #'null.waveguide.multiple_band.constant_spacing.wav',
        'waveguide.44100.wav',
        'raytracer.44100.wav',
        'waveguide.5000.wav',
        'raytracer.5000.wav',
    ]]

    max_frequency = 240 

    #analysis.modal_analysis(fnames, max_frequency, [5.56, 3.97, 2.81])
    analysis.modal_analysis(fnames, max_frequency)

if __name__ == '__main__':
    main()
