#!/usr/local/bin/python

import soundfile
import scipy.signal
import os.path
import numpy

def main():
    carrier = 'anechoic.wav'
    carrier_signal, sr = soundfile.read(carrier)

    files = [
            ('cardioid_away',       ['receivers/cardioid_away.wav']),
            ('concert',             ['receivers/concert.wav']),
            ('binaural',            ['receivers/left_ear.wav', 'receivers/right_ear.wav']),
            ('0.02',                ['room_materials/0.02.wav']),
            ('0.04',                ['room_materials/0.04.wav']),
            ('0.08',                ['room_materials/0.08.wav']),
            ('small',               ['room_sizes/small.wav']),
            ('medium',              ['room_sizes/medium.wav']),
            ('large',               ['room_sizes/large.wav']),
            ('large_spaced',        ['room_sizes/large_spaced.wav']),
            ('tunnel_no_scatter',   ['tunnel/tunnel_near_no_scatter.wav']),
            ('tunnel',              ['tunnel/tunnel_near.wav']),
            ('vault',               ['vault/vault.wav']),
        ]

    if not soundfile.check_format('WAV', 'PCM_24'):
        raise RuntimeError('That format is not valid')

    def convolve_channel(channel):
        signal, _ = soundfile.read(channel)
        return scipy.signal.fftconvolve(carrier_signal, signal)
    
    for name, channels in files:
        convolved = numpy.transpose(numpy.array([convolve_channel(channel) for channel in channels], dtype='float32'))
        convolved /= numpy.max(numpy.abs(convolved))
        soundfile.write('out_' + name + '.wav', convolved, sr, subtype='PCM_24', format='WAV')

if __name__ == '__main__':
    main()
