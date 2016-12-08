---
layout: page
title: Context
navigation_weight: 1
---

---
reference-section-title: References
...

# Context {.major}

<!--

Methods for room acoustic simulation
	Geometric
	Wave-based

	Real-time vs offline

	Advantages and disadvantages

Available software
	Advantages and disadvantages

	References

What is needed? What niche exists?

How have I decided to solve the problem?

-->

## Acoustics Simulation Techniques

### Overview

Room acoustics algorithms fall into two main categories: *geometric*, and
*wave-based* [@southern_spatial_2011].
Wave-based methods aim to numerically solve the wave equation, simulating the
actual behaviour of sound waves within an enclosure.
Geometric methods instead make some simplifying assumptions about the behaviour
of sound waves, which result in faster but less accurate simulations.
These assumptions generally ignore all wave properties of sound, choosing to
model sound as independent *rays*, *particles*, or *phonons*.

The modelling of waves as particles has found great success in the field of
computer graphics, where *ray-tracing* is used to simulate the reflections of
light in a scene.
The technique works well here because of the relatively high frequencies of the
modelled waves.
The wavelengths of these waves - the wavelengths of the visible spectrum - will
generally be many times smaller than any surface in the scene being rendered, so
wave phenomena have little or no effect.

The assumption that rays and waves are interchangeable falls down somewhat when
modelling sound.
Here, the wavelengths range from 17m to 0.017m for the frequency range
20Hz to 20KHz, so while the simulation may be accurate at high frequencies,
at low frequencies the wavelength is of the same order as the wall surfaces in
the scene.
Failure to take wave effects such as interference and diffraction into account
at these frequencies therefore results in noticeable approximation error
[@savioja_overview_2015].

In many cases, some inaccuracy is an acceptable (or even necessary) trade-off.
Wave-modelling is so computationally expensive that using it to simulate a
large scene over a broad spectrum could take weeks on consumer hardware.
This leaves geometric methods as the only viable alternative.
Though wave-modelling been studied for some time [@smith_physical_1992], and
even applied to small acoustic simulations in consumer devices (such as the
Yamaha VL1 keyboard), it is only recently, as computers have become more
powerful, that these techniques have been seriously considered for room
acoustics simulation.

Given that wave-based methods are accurate, but become more expensive at higher
frequencies, and that geometric methods are inexpensive, but become less
accurate at lower frequencies, it is natural to combine the two models in a way
that takes advantage of the desirable characteristics of each.
That is, by using wave-modelling for low-frequency content, and geometric
methods for high-frequency content, simulations may be produced which are
accurate across the entire spectrum, without incurring massive computational
costs.

### Characteristics of Simulation Methods

A short review of simulation methods will be given here.
For a detailed survey of methods used in room acoustics, see
@svensson_computational_2002.

TODO a diagram giving a nice overview.

#### Geometric

Geometric methods can largely be grouped into two categories: *stochastic* and
*deterministic*. 

Stochastic methods are generally based on statistical approximation via some
kind of Monte Carlo algorithm.
They may be based directly on reflection paths, using *ray tracing* or *beam
tracing*, in which rays or beams are considered to transport acoustic energy
around the scene.
Alternatively, they may use a surface-based technique, such as *acoustic
radiance transfer*, in which surfaces are used as intermediate stores of
acoustic energy.

These techniques are inherently approximate.
They aim to randomly probe the problem space repeatedly, combining the results
from several samples so that they converge upon the impulse response for a
scene.
They can be tuned easily, as quality can be traded-off against speed simply by
adjusting the number of samples taken.
Surface-based methods, especially, are suited to real-time, as the calculation
occurs in several passes, only the last of which involves the receiver object.
This means that early passes can be computed and cached, and only the final pass
must be recomputed if the receiver position changes.

The main deterministic method is the *image source* method, which is designed to
calculate the exact reflection paths between a source and a receiver.
For shoebox-shaped rooms, and perfectly rigid surfaces, it is able to
produce an exact solution to the wave equation.
However, by its nature, it can only model specular (perfect) reflections,
ignoring diffuse and diffracted components.
For this reason, it is inexact for arbitrary enclosures, and unsuitable for
calculating reverb tails, which are predominantly diffuse.
The technique also becomes very expensive beyond low orders of reflection.
The naive implementation reflects the sound source against all surfaces in the
scene, resulting in a set of *image sources*.
Then, each of these image sources is itself reflected against all surfaces.
For high orders of reflection, the required number of calculations quickly
becomes impractical.
For these reasons, the image source method is only suitable for early
reflections, and is generally combined with a stochastic method to find the
late part of an impulse response.

For a thorough exploration of geometric methods please refer to
@savioja_overview_2015.

#### Wave-based 

Wave-based methods may be derived from the *Finite Element Method* (FEM),
*Boundary Element Method* (BEM) or *Finite-Difference Time-Domain* (FDTD)
method.
Of these, the FEM and BEM operate in the frequency domain, while the FDTD
operates directly in the time domain, as its name suggests.

### Existing Software

Room acoustics simulation is not a new topic of research.
The first documented method for estimating a room impulse response was put
forward in [@krokstad_calculating_1968], which describes a geometric method
based on ray tracing.

TODO Since then... 

Searching online uncovers a handful of programs for acoustic simulation:

Name                                  | Type                        | Availability
--------------------------------------|-----------------------------|------------------
Odeon [@_odeon_2016]                  | Geometric                   | Commercial       
CATT-Acoustic [@_catt-acoustic_2016]  | Geometric                   | Commercial 
Olive Tree Lab [@_otl_2016]           | Geometric                   | Commercial
EASE [@_ease_2016]                    | Geometric                   | Commercial
Auratorium [@_audioborn_2016]         | Geometric                   | Commercial
RAVEN [@schroder_raven:_2011]         | Geometric                   | None
RoomWeaver [@beeson_roomweaver:_2004] | Waveguide                   | None
EAR [@_ear_2016]                      | Geometric                   | Free
PachydermAcoustic [@_pachyderm_2016]  | Geometric                   | Free
Parallel FDTD [@_parallelfdtd_2016]   | Waveguide                   | Free
i-Simpa [@_i-simpa_2016]              | Geometric, extensible       | Free
