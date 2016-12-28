---
layout: page
title: Hybrid
navigation_weight: 5
---

---
reference-section-title: References
...

# Hybrid {.major}

## Background

### Crossover Position

There is no concrete rule to place the crossover between "low" and "high"
frequencies in this context.  It should be clear that, when the time and
computing power is available, the cutoff should be placed as high as possible,
so as to use accurate wave-based modelling for the majority of the output.
However, in practice, it might be useful to have an estimate for the
frequencies where wave-modelling is strictly required, to guide the placement
of the cutoff frequency.  The *Schroeder frequency* is such an estimate, and is
based on the density of room resonances or "modes".  Below the Schroeder
frequency, room modes are well separated and can be individually excited.
Above, the room modes overlap much more, resulting in a more "even" and less
resonant sound.  The Schroeder frequency is defined as follows (see
[@kuttruff_room_2009, p. 84] for a detailed derivation):

(@) $$2000\sqrt{\frac{RT60}{V}}$$

Here, $RT60$ is the time taken for the reverb tail to decay by 60dB, and $V$ is
the room volume in cubic metres.  Note that the Schroeder frequency is
inversely proportional to the square root of the room volume.  This implies
that in larger rooms, the cutoff between modal and non-modal behaviour is
lower, and therefore wave-based methods will be required to compute a smaller
portion of the spectrum.  In turn, this helps to keep the computational cost of
wave-based simulations within reasonable bounds.  Although the cost of
wave-based methods increases with output frequency and simulation size,
required output frequency decreases with simulation size.  As a result, a
hybrid acoustic simulator should be able to simulate even very large enclosures
with reasonable accuracy, without incurring excessive costs.

