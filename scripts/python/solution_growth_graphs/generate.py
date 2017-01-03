#!/usr/local/bin/python

import soundfile as sf
import numpy as np
import scipy.signal
import matplotlib
render = True
if render:
    matplotlib.use('pgf')
import matplotlib.pyplot as plt

from mpl_toolkits.axes_grid1.inset_locator import zoomed_inset_axes
from mpl_toolkits.axes_grid1.inset_locator import inset_axes
from mpl_toolkits.axes_grid1.inset_locator import mark_inset

def main():
    plt.figure(figsize=(10, 12))
    plt.suptitle('The Effect of Excitation Signal on Solution Growth')

    signals = [
            ('Dirac', 'solution_growth.dirac.transparent.output.aif'),
            ('Sine-modulated Gaussian', 'solution_growth.sin_modulated_gaussian.transparent.output.aif'),
            ('Differentiated Gaussian', 'solution_growth.differentiated_gaussian.transparent.output.aif'),
            ('Ricker', 'solution_growth.ricker.transparent.output.aif'),
            ('PCS', 'solution_growth.pcs.soft.output.aif'),
    ]

    for i, (name, fname) in enumerate(signals, start=1):
        ax = plt.subplot(len(signals), 1, i)
        ax.set_title(name)
        ax.set_xlabel('time / samples')
        ax.set_ylabel('amplitude')

        data, samplerate = sf.read(fname)
        m = np.max(np.abs(data[:100]))
        data /= m

        ax.plot(data)

        axins = inset_axes(ax, width="70%", height="70%", loc=9)
        axins.set_xlim(1, 500)
        axins.set_ylim(-1, 1)
        axins.plot(data)

        mark_inset(ax, axins, loc1=2, loc2=4, fc="none", ec="0.5")

    plt.tight_layout()
    plt.subplots_adjust(top=0.9)

    plt.show()
    if render:
        plt.savefig(
            'solution_growth.svg',
            bbox_inches='tight',
            dpi=300,
            format='svg')


if __name__ == '__main__':
    pgf_with_rc_fonts = {
        'font.family': 'serif',
        'font.serif': [],
        'font.sans-serif': ['Helvetica Neue'],
        'font.monospace': ['Input Mono Condensed'],
        'legend.fontsize': 12,
    }

    matplotlib.rcParams.update(pgf_with_rc_fonts)

    main()
