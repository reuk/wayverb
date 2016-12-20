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

Similarly to the image-source method, ray tracing assumes that sound energy is transported around a scene in "rays".
The rays start at the sound source, and are emitted at the same time, travelling at the speed of sound.
When a ray hits a boundary, it loses some energy, and is reflected.
When it intersects the receiver, the energy and time-delay of the ray is recorded.
In these ways, the models are similar.
However, there are some important differences between the two methods, explained below.

### Stochastic Simulation

Image sources are deterministic, while ray tracing is stochastic.
The image-source method finds exact specular reflections, which will be constant for given source, receiver, and boundary positions.
Ray tracing is less accurate, aiming to compute a result which is correct, within a certain probability.
A large number of rays are fired into the scene in random directions, and reflected up to a certain depth.
Some of these rays may intersect with the receiver volume, but some may not.
Only rays that *do* intersect the receiver contribute to the final output.
The proportion of rays which intersect the receiver, and therefore the measured energy at the receiver, can be found with greater accuracy simply by increasing the number of rays fired.

### Receiver Volume

The random nature of ray tracing requires that the receiver must have a finite volume.
The likelihood of any given ray intersecting with a single point with no volume is zero.
If the probability of a ray-receiver intersection is to be non-zero, the receiver must have some volume.
This is different to image-source, which traces reflections backwards from the exact receiver position, allowing it to be represented as a point.

### Energy and Distance

In ray tracing, each ray represents a finite portion of the initial source energy.
The reduction of energy over a given distance is accounted-for by the spreading-out of the rays.
This can be illustrated very simply:
First, imagine a sphere placed very close to a point.
Assuming rays are fired with a uniform random distribution from that point, a certain proportion of those rays will intersect with the sphere.
If the sphere is moved further away, a smaller proportion of rays will hit it.
The exact proportion is equal to $\frac{s}{4r^2}$, where $s$ is the constant area covered by the receiver, and $r$ is the distance between the source and receiver.
That is, the proportion of rays intersecting the receiver is inversely proportional to the square of the distance between the source and receiver.
The energy recorded is proportional to the number of ray intersections recorded, therefore, the ray model intrinsically accounts for the inverse-square law for energy, and the per-ray energy does not need to be scaled proportionally to the distance travelled.
This is different to the image-source model, in which only valid specular reflections are recorded, and the inverse-square law may be applied directly.

TODO ray diagram

### Rendering

The final major difference between ray tracing and the image-source method is to do with the way in which results are recorded.
The image-source method finds exact specular reflections, each of which contributes an impulsive signal with specific frequency content at a precise time.
This reflection data is precise and accurate, so it can be used to render an output signal at arbitrarily high sampling frequencies.
Ray tracing, on the other hand, is inexact, based on statistical methods.
For a given unit time, the number of rays detected must be very high in order for the detected energy level to be accurate within certain bounds.
[@vorlander_auralization:_2007, p. 191] shows that the mean number of intersections $k$ per time period $\delta t$ is given by

$$k=\frac{N\pi r^2c\Delta t}{V}$$

where $N$ is the number of rays, $r$ is the radius of the receiver, $c$ is the speed of sound, and $V$ is the room volume.

For an output which covers the human hearing range, the sampling rate must be at least 40KHz, which corresponds to a sampling period of 25μs.
Therefore, for a receiver radius of 0.1m, and assuming 100 detections-per-second is adequate, the minimum number of rays is

$$N=\frac{kV}{\pi r^2c\Delta t} = \frac{100V}{340 \cdot 0.000025 \cdot 0.1^2 \pi} \approx 374500V$$

For higher accuracy, higher output sample rates, and smaller receivers the number of rays required becomes even greater.
Even on modern hardware, this sheer quantity of rays is prohibitive.

If, on the other hand, audio-rate results are not required, then the number of necessary rays is much lower.
[@vorlander_auralization:_2007, p. 186] suggests a sampling period of the order of magnitude of milliseconds, which requires at least 40-times fewer rays.

Now, the ray tracer can be thought to produce an *energy envelope*, describing the decay tail of the impulse response.
To produce the impulse response itself, this energy envelope is simply overlaid onto a noise-like signal.
The process will be described in greater detail in the following [Implementation] section.

## Implementation

Here, Wayverb's ray tracer will be described.
Details of the boundary- and microphone-modelling processes are discussed separately, in the [Boundary Modelling]({{ site.baseurl }}{% link boundary.md %}) and [Microphone Modelling]({{ site.baseurl }}{% link microphone.md %}) sections respectively.
The following image \text{(\ref{fig:ray_tracer_process})} outlines the entire process.

![Estimating the reverb tail using ray tracing.\label{fig:ray_tracer_process}](images/ray_tracer_process)

### Finding Reflections

The simulation begins identically to the image-source process.
A voxel-based acceleration structure is created, to speed up ray intersection tests.

Rays are fired in uniformly-distributed random directions from the source point.
Each ray carries a certain quantity of energy (the method for determining the starting energy is described in the [Hybrid]({{ site.baseurl }}{% link hybrid.md %}) section).
If a ray intersects with the scene geometry, some data is stored about that intersection:

* its position
* the unique id of the triangle which was intersected
* whether or not the receiver point is visible from this position

A new ray direction is calculated using the *vector-based scattering* method, described in [@christensen_new_2005].
A uniformly random vector is generated, within the hemisphere oriented in the same direction as the triangle normal.
The ideal specular direction is also calculated, and the two vectors are combined by $\overrightarrow{R}_\text{outgoing}=s\overrightarrow{R}_\text{random} + (1-s)\overrightarrow{R}_\text{specular}$, where $s$ is the scattering coefficient.
Normally, the scattering coefficient would be defined per-band, but this would require running the ray tracer once per band, so that each frequency component can be scattered differently.
Instead, the mean scattering coefficient is used, so that all bands can be traced in one pass.

Having calculated a new ray direction, the energy carried in the ray is decreased, depending on the absorption coefficients of the intersected triangle.
If the surface has per-band absorptions coefficients $\alpha$, then the energy in each band is multiplied by $(1 - \alpha)$ to find the outgoing energy.
The new ray, with the computed outgoing energy and vector-scattered direction, is now traced.

The ray tracing process continues for a set number of reflections.
Typically, each ray would be traced until the energy in all bands has fallen below a certain threshold, requiring an additional check per reflection per ray [@vorlander_auralization:_2007, p. 183].
Under such a scheme, some rays might reach this threshold faster than others, depending on the absorptions of intermediate materials.
However, in Wayverb all rays are traced in parallel, so it is not feasible or necessary to allow rays to quit early.
Instead, the maximum possible required depth is found before the simulation, and all rays are traced to this maximum depth.

To find the maximum ray tracing depth, first the minimum absorption of all surfaces in the scene is found.
The outgoing energy from a reflection is equal to $E_\text{incoming}(1-\alpha)$ where $E_\text{incoming}$ is the incoming energy and $\alpha$ is the surface absorption.
The maximum ray tracing depth is equal to the number of reflections from the minimally absorptive surface required to reduce the energy of a ray by 60dB:

$$n_\text{reflections}=\left\lceil-\frac{3}{\log_{10}(1-\alpha_\text{min})}\right\rceil$$

### Logging Energy

The output of the ray tracing process is a histogram, plotting recorded energy per time step.
This recorded energy may come from two different sources.

Firstly, if a ray intersects with the receiver volume, then the current energy of that ray, which may have been attenuated by previous reflections, is added to the histogram at the appropriate time step.
The time of the energy contribution is given by the total distance travelled by the ray, divided by the speed of sound.
This is the approach taken in typical acoustic ray tracers.

Secondly, each reflection point is considered to spawn a "secondary source" which emits scattered sound energy.
If the receiver is visible from the reflection point, then a small energy contribution is logged, at a time proportional to the distance travelled by the ray.
The exact level of this contribution is explained in the Geometric Implementation subsection of the [Boundary Modelling]({{ site.baseurl }}{% link boundary.md %}) page.

### Producing Audio-rate Results

When ray tracing has completed, the result is a set of histograms which describe the energy decay envelope of each frequency band.
These histograms will have a relatively low sampling rate of around 1KHz, as explained above, so they are not directly suitable for auralisation.
To produce audio-rate impulse responses, the "fine structure" of the decay tail must be synthesised and then enveloped using the histogram envelopes.
The process used to convert the histogram into an audio-rate impulse response is described in greater depth in [@heinz_binaural_1993], though an overview will be given here.

First, a noise-like sequence of dirac impulses is generated, at audio-rate.
This sequence is designed to mimic the density of reflections in an impulse response of a certain volume
Therefore it starts sparse, and the density of impulses increases with time.

TODO describe poisson noise

The noise signal is split into frequency bands, matching the frequency bands used in the ray tracing simulation.

TODO describe multiband filter

Then, each band can be enveloped using the histogram for that band.
The enveloping is not quite as simple as multiplying each noise sample with the histogram entry at the corresponding time.
Instead, the enveloping process must conserve the energy level recorded for that time step.
This requires that the noise signal amplitude is also adjusted depending on the number of dirac events during the time-step.

TODO describe enveloping process

The final broadband signal is found by summing together the weighted noise sequences.

TODO nice pictures which are nice
