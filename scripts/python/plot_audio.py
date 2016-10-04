import matplotlib.pyplot as plt
import numpy as np
import wave
import sys

def main():
    spf = wave.open("/home/reuben/dev/waveguide/build/sndfile.wav")

    signal = spf.readframes(-1)
    signal = np.fromstring(signal, "Int16")

    NFFT = 1024
    Fs = spf.getframerate()
    dt = 1.0 / Fs

    t = [i * dt for i in range(len(signal))]

    ax0 = plt.subplot(211)
    plt.plot(t, signal)

    ax1 = plt.subplot(212, sharex=ax0)
    plt.specgram(signal, NFFT=NFFT, Fs=Fs, noverlap=NFFT/2)

    plt.show()

if __name__ == "__main__":
    main()
