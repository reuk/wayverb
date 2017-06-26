---
layout: page
title: Ray tracer
navigation_weight: 3
---

--- 
reference-section-title: References 
...

# Ray Tracer {.major}

## Background

Similarly to the image-source method, ray tracing assumes that sound energy
travels around a scene in "rays".  The rays start at the sound source, and are
all emitted in uniformly random directions at the same time, travelling at the
speed of sound.  When a ray hits a boundary, it loses some of its energy,
depending on the properties of the boundary's material. Then, the ray is
reflected.  When it intersects the receiver, the energy and time-delay of the
ray is recorded.  In these ways, the models are similar.  However, there are
some important differences between the two methods, explained below.

### Stochastic Simulation

Image sources are deterministic, while ray tracing is stochastic.  The
image-source method finds exact specular reflections, which are fixed for given
source, receiver, and boundary positions.  Ray tracing is less accurate, aiming
to compute a result which is correct, within a certain probability.  A large
number of rays are fired into the scene in random directions, and traced until
they have undergone a certain number of reflections.  Some of these rays may
intersect with the receiver volume (see below), but some may not.  Only rays
that *do* intersect the receiver contribute to the final output.  The
proportion of rays which intersect the receiver, and therefore the measured
energy at the receiver, can be found with greater accuracy simply by increasing
the number of rays fired.

### Receiver Volume

The random nature of ray tracing requires that the receiver must have a finite
volume.  The likelihood of any given random ray intersecting with a single
point with no volume tends to zero (there is an infinite number of possible ray
directions, only one of which passes through the point).  If the probability of
a ray-receiver intersection is to be non-zero, the receiver must have some
volume.  This is different to the image-source method, which traces reflections
backwards from the receiver, allowing it to be represented as a point.

### Energy and Distance

In ray tracing, each ray represents a finite portion of the initial source
energy.  The reduction of energy over a given distance is accounted for by the
spreading-out of the rays.  This can be illustrated very simply: Imagine a
sphere placed very close to a point.  Assuming rays are fired with a uniform
random distribution from that point, a certain proportion of those rays will
intersect with the sphere.  If the sphere is moved further away, a smaller
proportion of rays will hit it (see +@fig:receiver_proximity).

![The proportion of randomly-distributed rays intersecting with a sphere
depends on the distance between the ray source and the
sphere.](images/receiver_proximity){#fig:receiver_proximity}

The exact proportion of intersecting rays is equal to $s/4r^2$
[@schroder_physically_2011, p. 75], where $s$ is the constant area covered by
the receiver, and $r$ is the distance between the source and receiver.  That
is, the proportion of rays intersecting the receiver is inversely proportional
to the square of the distance between the source and receiver.  The energy
registered is proportional to the number of ray intersections recorded,
therefore the ray model intrinsically accounts for the inverse-square law for
energy, and the per-ray energy does not need to be scaled proportionally to the
distance travelled.  This differs to the image-source model, in which only
valid specular reflections are recorded, and the inverse-square law must be
applied directly.

### Rendering

The final major difference between ray tracing and the image-source method is
to do with the way in which results are recorded.  The image-source method
finds exact specular reflections, each of which contributes an impulsive signal
with specific frequency content at a precise time.  This reflection data is
precise and accurate, so it can be used to render an output signal at
arbitrarily high sampling frequencies.  Ray tracing, on the other hand, is
inexact because it is based on stochastic methods.  The accuracy of the output
increases with the average number of rays detected per unit time.  It is shown
in [@vorlander_auralization:_2007, p. 191] that the mean number of
intersections $k$ per time period $\Delta t$ is given by

$$k=\frac{N\pi r^2c\Delta t}{V}$$ {#eq:}

where $N$ is the number of rays, $r$ is the radius of the receiver, $c$ is the
speed of sound, and $V$ is the room volume.

For an output which covers the human hearing range, the sampling rate must be
at least 40kHz, which corresponds to a sampling period of 25$\mu$s.
Therefore, for a receiver radius of 0.1m (around the size of a human head), and
assuming that one detection-per-sampling-period is adequate, the minimum number
of rays is

$$N=\frac{kV}{\pi r^2c\Delta t} = \frac{V}{\pi \cdot 0.1^2 \cdot 340 \cdot
0.000025 } \approx 3745V$$ {#eq:}

In actual simulations, especially of large and complex rooms, this number of
rays is likely to produce results with large, inaccurate energy fluctuations.
For higher accuracy, higher output sample rates, and smaller receivers the
number of rays required becomes even greater.  This sheer quantity of rays
requires a vast number of independent calculations which will be prohibitively
time-consuming, even on modern hardware.

If, on the other hand, audio-rate results are not required, then the number of
necessary rays is much lower.  Vorländer suggests a sampling period of the
order of magnitude of milliseconds, which requires at least 40-times fewer rays
[@vorlander_auralization:_2007, p. 186].

Now, the ray tracer can be thought to produce an *energy envelope* describing
the decay tail of the impulse response.  To produce the impulse response
itself, this energy envelope is simply overlaid onto a noise-like signal.  The
process will be described in greater detail in the following [Implementation]
section.

## Implementation

Here, Wayverb's ray tracer will be described.  Details of the boundary- and
microphone-modelling processes are discussed separately, in the [Boundary
Modelling]({{ site.baseurl }}{% link boundary.md %}) and [Microphone
Modelling]({{ site.baseurl }}{% link microphone.md %}) sections respectively.

### Finding Reflections

The simulation begins identically to the image-source process.  A voxel-based
acceleration structure is created, to speed up ray intersection tests.

Rays are fired in uniformly-distributed random directions from the source
point.  Each ray carries an equal quantity of energy (the method for
determining the starting energy is described in the [Hybrid]({{ site.baseurl
}}{% link hybrid.md %}) section).  If a ray intersects with the scene geometry,
data is stored about that intersection: its position, the unique ID of the
triangle which was intersected, and whether or not the receiver point is
visible from this position.  This data will be used later on, when calculating
energy loss, and the directional distribution of received energy.

Next, a new ray direction is calculated using the *vector-based scattering*
method, described by Christensen and Rindel [@christensen_new_2005].  A
uniformly random vector is generated, within the hemisphere oriented in the
same direction as the triangle normal.  The ideal specular direction is also
calculated, and the two vectors are combined by

$$\overrightarrow{R}_\text{outgoing}=s\overrightarrow{R}_\text{random} +
(1-s)\overrightarrow{R}_\text{specular}$$ {#eq:}

where $s$ is the scattering coefficient.  Normally, the scattering coefficient
would be defined per-band, but this would require running the ray tracer once
per band, so that each frequency component can be scattered differently.
Instead, the mean scattering coefficient is used, so that all bands can be
traced in one pass. For eight frequency bands, this provides an eight-times
speed-up, at the cost of inaccurate interpretation of the scattering
coefficients. This is a reasonable trade-off, as scattering will also be
modelled using the *diffuse rain* technique described in the [Boundary
Modelling]({{ site.baseurl }}{% link boundary.md %}) section, which *is* able
to account for different per-band scattering coefficients. The final output
will therefore retain per-band scattering characteristics, but with much
improved performance.

Having calculated a new ray direction, the energy carried in the ray is
decreased, depending on the absorption coefficients of the intersected
triangle.  If the surface has an absorption coefficient of $\alpha$ in a
particular band, then the energy in that band is multiplied by $(1 - \alpha)$
to find the outgoing energy.  This process is repeated, using the incoming
energy and absorption coefficient for each band, to find outgoing energies in
all bands. The new ray, with the computed outgoing energies and
vector-scattered direction, is now traced.

The ray tracing process continues for a set number of reflections.  Typically,
each ray would be traced until the energy in all bands has fallen below a
certain threshold, requiring an additional check per reflection per ray
[@vorlander_auralization:_2007, p. 183].  Under such a scheme, some rays might
reach this threshold faster than others, depending on the absorptions of
intermediate materials.  However, in Wayverb all rays are traced in parallel,
so it is not feasible or necessary to allow rays to quit early. The time taken
for a parallel computation will always be limited by the longest-running
process. If some rays are "stopped" early, this does not improve the
processing-speed of the continuing rays, so the simulation still takes the same
time to complete.  Instead, the maximum possible required depth is found before
the simulation, and all rays are traced to this maximum depth.

To find the maximum required ray tracing depth, first the minimum absorption of
all surfaces in the scene is found.  The outgoing energy from a single
reflection is equal to $E_\text{incoming}(1-\alpha)$ where $E_\text{incoming}$
is the incoming energy and $\alpha$ is the surface absorption.  It follows that
the outgoing energy from a series of reflections is given by
$E_\text{incoming}(1-\alpha)^{n_\text{reflections}}$.  Then, the maximum ray
tracing depth is equal to the number of reflections from the minimally
absorptive surface required to reduce the energy of a ray by 60dB:

$$(1-\alpha_\text{min})^{n_\text{reflections}} = 10^{-6}
\therefore
n_\text{reflections}=\left\lceil-\frac{6}{\log_{10}(1-\alpha_\text{min})}\right\rceil$$ {#eq:}

The 60dB level decrease is somewhat arbitrary, but was chosen to correspond to
the *RT60*, which is a common descriptor of recorded impulse responses. The
RT60 is a measure of reverb length, defined as the time taken for the sound
level to decrease by 60dB. In a future version of the software, the level
decrease might be set depending on the dynamic range of the output format. This
would allow 16-bit renders (with around 48dB of dynamic range) to use fewer
reflections, while 32-bit outputs with lower noise floors would require more
reflections.

### Logging Energy

The output of the ray tracing process is a histogram, plotting recorded energy
per time step.  This recorded energy may come from two different sources.

Firstly, if a ray intersects with the receiver volume, then the current energy
of that ray, which may have been attenuated by previous reflections, is added
to the histogram at the appropriate time step.  The time of the energy
contribution is given by the total distance travelled by the ray, divided by
the speed of sound.  This is the approach taken in typical acoustic ray
tracers.

Secondly, each reflection point is considered to spawn a "secondary source"
which emits scattered sound energy, depending on the scattering coefficients of
the surface.  If the receiver is visible from the reflection point, then a
small energy contribution is logged, at a time proportional to the distance
travelled by the ray.  This mimics the real world behaviour of rough surfaces,
which cause some energy to be randomly diffused in non-specular directions
during reflection of the wave-front. The exact level of this contribution is
explained in the Geometric Implementation subsection of the [Boundary
Modelling]({{ site.baseurl }}{% link boundary.md %}) page.

### Producing Audio-rate Results

When ray tracing has completed, the result is a set of histograms which
describe the energy decay envelope of each frequency band.  These histograms
will have the relatively low sampling rate, as explained above (Wayverb uses a
sampling rate of 1kHz).  As a result, these histograms are not directly
suitable for auralisation.  To produce audio-rate impulse responses, the "fine
structure" of the decay tail must be synthesised and then the gain adjusted
using the histogram envelopes.  The process used in Wayverb to convert the
histogram into an audio-rate impulse response is described in
[@heinz_binaural_1993], and in greater depth in [@schroder_physically_2011, p.
70], though an overview will be given here.  *@fig:raytrace_process outlines
the process of estimating an audio-rate representation of low-sample-rate
multi-band histograms.

![Generating an audio-rate signal from multi-band ray tracing energy histograms
at a low sampling rate.](images/raytrace_process){#fig:raytrace_process}

#### Generating a Noise Signal

First, a noise-like sequence of Dirac impulses is generated at audio-rate.
This sequence is designed to mimic the density of reflections in an impulse
response of a certain volume.  Therefore it is modelled as a temporal Poisson
process which starts sparse, and with increasing density of impulses over time.
Specifically, the time between one impulse event and the next is given by

$$\Delta t_\text{event}(z) = \frac{\ln\frac{1}{z}}{\mu}$$ {#eq:}

where $z$ is a uniformly distributed random number $0 < z \leq 1$.  $\mu$ here
is the mean event occurrence, and is dependent upon the current simulation time
$t$, the enclosure volume $V$ and the speed of sound $c$:

$$\mu = \frac{4\pi c^3 t^2}{V}$$ {#eq:}

It can be seen that the mean occurrence is proportional to the square
of the current time, producing an increase in event density over time.  The
first event occurs at time $t_0$:

$$t_0=\sqrt[3]{\frac{2V\ln 2}{4\pi c^3}}$$ {#eq:}

The full-length noise signal is produced by repeatedly generating inter-event
times $\Delta t_\text{event}$, and adding Dirac impulses to a buffer, until the
final event time is greater or equal to the time of the final histogram
interval.  Dirac deltas falling on the latter half of a sampling interval are
taken to be negative-valued.  The number of Dirac deltas per sample is limited
to one, and the value of $\mu$ is limited to a maximum of 10kHz, which has been
shown to produce results absent of obvious artefacts [@heinz_binaural_1993].

#### Weighting Noise Sequences

The noise sequence is duplicated, once for each frequency band.  Then, the
noise sequence for each band is weighted according to that band's histogram.
This enveloping is not quite as simple as multiplying each noise sample with
the histogram entry at the corresponding time.  Instead, the enveloping process
must conserve the energy level recorded over each time step.

For each time interval in the histogram, the corresponding range of samples in
the noise sequence is found.  If the output sample rate is $f_s$ and the
histogram time step is $\Delta t$, then the noise sequence sample corresponding
to histogram step $h$ is $\lfloor h \cdot f_s \cdot \Delta t \rfloor$.  The
corrected energy level for each histogram step is found by dividing the
histogram energy value by the sum of squared noise samples for that step.  This
is converted to a corrected pressure level by $P = \sqrt{Z_0 I}$, where $I$ is
the corrected energy level, and $Z_0$ is the acoustic impedance of air.  The
weighting is now accomplished by multiplying each noise sequence sample
corresponding to this histogram step by the corrected pressure level.

#### Multi-band Filtering

Now, we are left with a set of broadband signals, each with different
envelopes.  The output signal is found by bandpass filtering each of these
signals, and then mixing them down.

The filter bank should have perfect reconstruction characteristics: a signal
passed through all filters in parallel and then summed should have the same
frequency response as the original signal. In the case where all the materials
in a scene have the same coefficients in all bands, the input to each filter
would be identical. Then, the expected output would be the same as the input to
any band (though band-passed between the lower cutoff of the lowest band and
the upper cutoff of the highest band).  Perfect-reconstruction filters maintain
the correct behaviour in this (unusual) case. It is especially important that
the bandpass filters are zero-phase, so that Dirac events in all bands are
in-phase, without group delay, after filtering. Finally, the filters should
have slow roll-off and no resonance, so that if adjacent bands have very
mismatched levels, there are no obvious filtering artefacts.

An efficient filtering method is to use a bank of infinite-impulse-response
filters.  These filters are fast, and have low memory requirements. They can
also be made zero-phase when filtering is offline, by running the filter
forwards then backwards over the input signal (though this causes filter
roll-off to become twice as steep). This was the initial method used in
Wayverb: the filter bank was constructed from second-order Linkwitz-Riley
bandpass filters. This method had two main drawbacks: the roll-off is limited
to a minimum of 24 dB/octave [@linkwitz_active_1976], which may cause an
obvious discontinuity in level in the final output; and the forward-backward
filtering method requires computing the initial filter conditions in order to
maintain perfect-reconstruction, which is non-trivial to implement
[@gustafsson_determining_1994].

A better method, which allows for shallower filter roll-offs while retaining
perfect-reconstruction capabilities is to filter each band directly in the
frequency domain.  The filtering of each signal is accomplished by computing
the signal's frequency-domain representation, attenuating bins outside the
passband, and then transforming the altered spectrum back to the time domain.
To ensure perfect reconstruction, and to avoid artificial-sounding
discontinuities in the spectrum, the filter shape suggested in
[@antoni_orthogonal-like_2010] is used.  This paper suggests equations which
describe the band-edge magnitudes:

$$
G_\text{lower}(\omega_\text{edge} + p) = \sin^2\left(\frac{\pi}{2}\phi_l(p)\right), \\
G_\text{upper}(\omega_\text{edge} + p) = \cos^2\left(\frac{\pi}{2}\phi_l(p)\right)
$$ {#eq:}

Here, $G$ is a function of frequency, $\omega_\text{edge}$ is the band-edge
frequency, and $p$ is the relative frequency of a nearby frequency bin.  The
equations are computed for a range of values $p=P,\dots,P$ where $P$ is the
width of the crossover.  The definition of $\phi_l(p), l \geq 0$ is recursive:

$$
\phi_l(p)=
\begin{cases}
	\frac{1}{2}(p / P + 1), & l = 0 \\
	\sin(\frac{\pi}{2}\phi_{l-1}(p)), & \text{otherwise} 
\end{cases}
$$ {#eq:}

The variable $l$ defines the steepness of the crossover, and is set to 0 in
Wayverb, so that the transition between bands is as slow and smooth as
possible.  The absolute width of the crossover is denoted by $P$, but it is
more useful to specify the crossover width in terms of overlap $0 \leq o \leq
1$.  Assuming logarithmically-spaced frequency bands, spread over the range
$\omega_\text{lowest}, \dots, \omega_\text{highest}$ where the edge frequency
of band $i$ is defined as

$$\omega_{\text{edge}_i}=\omega_\text{lowest}\left(\frac{\omega_\text{highest}}{\omega_\text{lowest}}^\frac{i}{N_\text{bands}}\right)$$ {#eq:}

the maximum width factor $w$ is given by

$$w=\frac{x-1}{x+1},
x=\frac{\omega_\text{highest}}{\omega_\text{lowest}}^\frac{1}{N_\text{bands}}$$ {#eq:}

For $\omega_\text{lowest}=20\text{Hz}$, $\omega_\text{highest}=20\text{kHz}$,
and $N_\text{bands}=8$, $w \approx 0.4068$.  Then, the band edge width $P$ can
be defined in terms of the overlap-amount $o$, the frequency of this band edge
$\omega_\text{edge}$, and the maximum allowable width factor $w$:
$P=\omega_\text{edge}ow$.  Wayverb sets the overlap factor $o=1$ to ensure
wide, natural-sounding crossovers.

The final broadband signal is found by summing together the weighted, filtered
noise sequences.
