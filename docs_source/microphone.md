---
layout: page
title: Microphone modelling
navigation_weight: 7
---

---
reference-section-title: References
...

# Microphone Modelling {.major}

## Introduction

In the preceding sections, simulation results have been recorded using a
virtual omnidirectional microphone model.  This model records the sound
pressure at a single point in space. The gain and frequency response of the
receiver are not affected by the direction of the pressure gradient.  Signals
recorded with omnidirectional microphones are primarily useful for assessing
the overall "character" of an acoustic space, including reverb time,
intelligibility, and overall gain.

However, one of the goals of room acoustics simulation is *virtual acoustics*:
recreating the modelled auditory environment in such a way that the listener
believes they are hearing sounds within a physical space. Real reverbs are
highly directional, containing direct sound from the source, and reflected
sounds from the room boundaries. To produce a convincing virtual environment,
these directional cues must be encoded, by adjusting the output gain and
frequency response depending on the direction of the cue. This is achieved
using a direction-sensitive receiver model.

## Background

Humans are able to detect the originating direction of sounds using two main
techniques: Interaural Time Difference (ITD) defines the time delay between
ears, when the sound source is located nearer to one ear than the other; and
Interaural Level Difference (ILD) denotes the difference in sound level (which
may vary by frequency) caused by attenuation by the outer ear, head, and torso.
The suitability of impulse responses for stereo or multichannel playback
depends on the ability to produce signals with appropriate ITD and ILD, in such
a way that each reflection is perceived as coming from a certain direction.

When recording impulse responses of physical spaces, several techniques might
be used to retain ITD and ILD information.

One option is to record the impulse response using a matched pair of
microphones.  An AB pair of spaced omnidirectional capsules will capture
interchannel time difference, but will only capture interchannel level
difference if the source is positioned near to the microphones. Alternatively,
an XY or Blumlein pair (which consist of coincident cardioid or bidirectional
capsules respectively) will capture level difference, but will be incapable of
recording time difference because wave-fronts will always arrive at both
capsules simultaneously.

Microphone pair methods are only suitable for recording stereo signals, as they
only capture a two-dimensional "slice" through the modelled scene, where all
directional information is restricted to the same plane as the microphone
capsules. The technique can be extended to higher dimensions by using more
microphone capsules. This is the basis of the ambisonic approach, which for
B-format recordings uses four coincident microphone capsules, three
bidirectional and one omnidirectional, to capture the three-dimensional
directional pressure gradient and overall pressure level.  Instead of being
used directly, the recorded signals are post-processed depending on the
configuration of the output speakers. For playback on headphones, the signals
can be filtered with *head related transfer functions* (HRTFs), which modify
the frequency content of the sound depending on its originating direction,
mimicking the absorptive characteristics of the human head and torso
[@noisternig_3d_2003].

If it is known that the recording will only be reproduced on headphones, the
preferred method for capturing impulse responses which retain appropriate ILD
and ITD is to use in-ear microphones, or a "dummy head" microphone pair.
Recordings made in this way capture the ITD and ILD that are caused by sonic
interactions with the outer ear, head, and torso, and produce a convincing
reconstruction of the sonic space when played back over headphones. However,
responses recorded using this technique are unsuitable for loudspeaker
playback.

With the exception of the spaced-pair methods, all of these techniques require
that the receiver gain is dependent upon the incident direction. The dummy head
technique additionally requires that the frequency response of the receiver is
dependent upon the incident direction.  To allow reproduction on both
headphones and arbitrary loudspeaker arrays, the receiver model in Wayverb
should encompass all of the techniques described above. Given that spaced
microphone techniques can be modelled simply by including multiple receivers in
the simulation, each virtual capsule should have configurable
direction-dependent gain and frequency response, allowing it to behave like a
standard microphone, or like the ear of a dummy head.

## Calculating Directional Gain

Wayverb contains two different models for calculating an appropriate gain from
an incident direction, described here.

### Microphone

The first method, the "perfect microphone", is very simple.  Given that the
virtual microphone is pointing in direction $\hat{p}$, and has a "shape"
parameter $0 \leq s \leq 1$, the direction-dependent attenuation $a(\hat{d})$
from direction $\hat{d}$ is given by

$$a(\hat{d}) = (1 - s) + s(\hat{d} \cdot \hat{p})$$ {#eq:}

where $\hat{p}$ and $\hat{d}$ are unit vectors, and $\cdot$ is the dot-product
operator. When $s$ is set to 0, the $a(\hat{d})$ is equal to one for all values
of $\hat{d}$, modelling an omnidirectional polar pattern. When $s$ is 1, the
modelled polar pattern is bidirectional, with a gain of 1 in front of the
microphone, 0 at either side, and -1 behind. When $s$ is 0.5, the pattern is
cardioid. Sub- and supercardioid patterns are created by setting $s$ lower or
higher than 0.5 respectively. This microphone model is flexible, allowing
almost all standard microphone techniques (including the XY pair, Blumlein
pair, and B-format ambisonic) to be simulated.

Note that this model has an ideally flat frequency response in every direction,
so it can only be used to model perfect microphones. It is not suitable for
modelling physical microphones with direction-dependent frequency responses. In
particular, with this technique it is not possible to produce binaural
dummy-head-style recordings, for immersive headphone playback.

### HRTF

A more general microphone model would allow specific per-frequency-band gains
to be set for each incoming direction.  Such a model is implemented in Wayverb,
but is used only for the modelling of HRTF characteristics. In the future, the
approach could easily be extended to simulate general non-flat microphones.

The method itself is based on a two-dimensional lookup table, indexed by the
azimuth and elevation angles formed by the incident direction. The table is
produced using experimentally-obtained HRTF data from the IRCAM Listen database
[@oliver_listen_2003]. The data takes the form of a collection of stereo audio
files, containing the impulse responses measured using microphones placed in
the left and right ears of a human volunteer. The recordings are made in an
anechoic environment. Each file contains the response measured for an
impulse originating at a specific azimuth and elevation.

To prepare the HRTF database for use in Wayverb, the audio files are
multi-band-filtered, using the same 8-band frequencies used throughout Wayverb.
Then, the per-band per-channel energy is recorded to a lookup table.

To find the receiver frequency response for a given incident direction, the
direction vector is converted to an azimuth and an elevation.  Then, this
azimuth and elevation are rounded to the closest angles which exist in the
lookup table.  They are used to index the lookup table, and the energies at
that index are returned.

This model contains two approximations: the first is the directional
discretisation process, which "quantises" incoming angles to a set of discrete,
equally-spaced bins.  In the final implementation, fifteen-degree increments
are used, matching the directional resolution of the experimentally-obtained
data.  However, humans can localise sounds from frontal sources to an accuracy
of one degree [@schroder_physically_2011, p. 22], so higher directional
resolution may be required to produce a convincing binaural effect. This
resolution is of course limited by the directional resolution of the input
data, and HRTF data with one-degree resolution would be time-consuming to
measure, and expensive to store.

The second approximation is in the frequency resolution of the lookup table.
HRTF data is supplied as a set of finite-impulse-response filter kernels, which
represent continuous frequency spectra, up to the sampling frequency of the
kernels. Generally, these filter kernels would be used via convolution,
resulting in a precise adjustment of the frequency content of the filtered
signal. If this approach were to be taken in Wayverb, it would require creating
a separate output signal for each of the incident directions, and then
convolving each direction separately, before a final mixdown. This would be
extremely costly: for separation angles of fifteen degrees, this means
recording signals for each of 288 incident directions (24 azimuth angles, each
with 12 elevation angles), which might easily occupy more than a gigabyte of
memory (288 3-second impulse responses at 32-bits and 44.1kHz require 1160MB of
memory), and then separately convolving and mixing them. While this is
definitely possible, it would be extremely time consuming to run. By
restricting the HRTF data to 8 frequency bands, the application architecture
can be simplified, while also improving memory usage and runtime performance.

## Image Source Implementation

As described in the [Image Source]({{ site.baseurl }}{% link image_source.md
%}) section, the magnitude of the contribution of each image source depends on
the distance between the source and receiver, and the direction-dependent
reflectance characteristics of any intermediate reflective surfaces. To model a
directional receiver, the contribution magnitude is additionally scaled by the
receiver's gain in the direction of that contribution.

The direction of each image source is found by subtracting the receiver
position from the image source position, and normalising the result.  This
direction can either be used to directly compute the appropriate attenuation
factor, or it can be used to look up the attenuation in a pre-computed table
(see [Calculating Directional Gain] below).

In the case that the frequency response (rather than overall gain) of the
receiver depends on incident direction, a different attenuation function or
look-up table is used for each frequency band. At the end of the simulation,
the outputs in each band are band-pass filtered and mixed down.

Wayverb includes a fast estimator for ITD in the image source model, which has
not been encountered in the literature.  During the mix-down process for the
image-source contributions, the time of each contribution is calculated based
on the distance from the image source to the receiver.  When calculating these
distances for the left ear, the receiver position can be temporarily moved 10cm
to the left, relative to the receiver orientation, and the same can be done for
the right ear.  The output signal will then incorporate small time delays
between channels, depending on the distance and direction of each image source.
This method relies on the assumption that all image sources are visible from
both ear positions, and that the new ear positions do not affect the
angle-dependent reflection characteristics of intermediate surfaces.  This is a
reasonable assumption, given that the distance between ears is generally small
compared to the size of the modelled room. The benefit of this method is that
the costly image-source-finding process only has to be run once in total,
instead of once for each ear.

This image-source "offsetting" technique cannot be applied to Wayverb's other
simulation methods. In the waveguide, the new "ear" positions may not fall
directly on grid nodes, and so the waveguide simulation would have to be run
twice, with different grids. The raytracer would not benefit from interaural
time difference at all, as the maximum possible interaural time delay is the
interaural distance divided by the speed of sound, 0.2/340=0.0005s, which is
less than the 1ms energy histogram sampling interval. Therefore, any measured
ITD would be lost when quantising to the histogram interval.

The drawback of the offsetting method is that, if the user manually positions
two receivers with appropriate interaural spacing, an extra offset will still
be automatically applied. This will lead to an interaural spacing that is twice
as wide as desired. This could be addressed by including an option in the
software to enable or disable the additional offset. Due to time constraints
this feature has not been implemented in the current version of Wayverb.

## Ray Tracer Implementation

In the ray tracer, a very similar approach could be taken.  The incident
direction of each ray is known, so this direction might be used with a function
or lookup-table to adjust the ray's multiband intensity before it is added to
the energy histogram (see [Ray Tracer]({{ site.baseurl }}{% link ray_tracer.md
%})).

In terms of implementation, this approach would be needlessly expensive.  In
Wayverb, the initial simulation and directional processing stages are kept
separate, allowing a single simulation to be processed by several different
virtual microphone capsules. While flexible, it is also infeasible to store the
direction of each incident ray, especially if there are hundreds of thousands
of rays in the simulation. The ray tracer is used to estimate the late
reverberation of the space, in which individual sonic events are "blurred"
together, and in which there are few directional cues
[@schroder_physically_2011, p. 21], so this level of per-ray directional
accuracy is unnecessary.

Instead, the surface of the spherical receiver is divided into discrete
*directivity groups*, mirroring the approach of Schroder in
[@schroder_physically_2011, p. 72]. A separate multiband energy histogram is
maintained for each directivity group, where the direction of ray incidence
determines the histogram used to store that ray's energy.  These histograms are
at a substantially lower sampling-frequency than the output sampling rate, so
it is feasible to store one for each discrete incident direction, as long as
the direction quantisation intervals are on the order of ten to twenty degrees.
To generate audio-rate results, the directional histograms must be combined
into a single energy histogram, which can then be used to weight a noise
sequence as discussed in the [Ray Tracer]({{ site.baseurl }}{% link
ray_tracer.md %}}) section. This is achieved by taking the centre direction of
each directivity group, calculating or looking-up the correct attenuation for
that direction, and then multiplying the entire histogram by that value. The
final direction-weighted histogram is given by the elementwise sum of all
individual weighted histograms.

<!-- TODO tests? -->

## DWM Implementation

### Implementation Decisions

Implementing directional receivers in the waveguide mesh is more complicated
than for the geometric methods.  Although absolute pressure values are
available in the mesh, directional pressure values are not inherently
supported.  In order to simulate a directional receiver, the instantaneous
direction of the sound field must be found at the receiver point.  Several
methods were considered, as outlined below.

#### Blumlein Difference Technique

The first method, described in [@southern_methods_2007] and
[@southern_2nd_2007], is designed with the goal of encoding the 3D sound-field
by means of spherical harmonic decomposition to the second order, for use in
ambisonic reproduction.

This method is based on sound intensity probe theory, which allows directional
velocity components of a sound field to be captured using two closely-spaced
omnidirectional receivers.  In the DWM, each node has an associated pressure,
so two closely-spaced nodes can be used to emulate a pair of omnidirectional
receivers, and in turn used to generate directional velocity components.  A
full first-order B-format signal can be made using seven such "pressure
sensors", arranged in a cross shape, with a single node at the centre, and six
nodes placed around it.  The pressure field velocity components are found by
subtracting nodes on "opposite" sides of the cross, while the centre node
contributes the overall pressure at the receiver. A second-order output
is achieved by adding further pressure-sensing nodes in between the first-order
nodes.

Note that this technique does not allow the immediate production of directional
responses. Instead, it is designed to produce outputs in ambisonic format,
which are then post-processed to create a signal with the desired modelled
directional response.

Additionally, this technique has some difficulty adapting to meshes with large
grid spacings.  In a rectilinear mesh, the seven first-order nodes can be
placed exactly, as their positions will coincide with nodes in the mesh.
However, the second-order nodes will not fall exactly on existing nodes, and
instead their positions will need to be quantised to the closest mesh nodes.
This problem becomes worse in other mesh topologies, such as the tetrahedral
mesh, where it may be that very few of the sensor nodes can be exactly placed
in the desired positions.

To facilitate more exact placement of sensor nodes, the spatial sampling
frequency of the mesh must be increased (by reducing the grid spacing, the
quantisation error in the sensor placement is also reduced).  The example given
in [@southern_2nd_2007] uses a sensor spacing of 0.04 metres, giving a maximum
valid sampling frequency of 4.25kHz.  However, to allow accurate placement of
these sensors, a grid with a spacing of 0.0027m is required.  This increase in
mesh density necessitates more than 3000 times the original number of nodes,
which is only practical for very small modelled spaces.  An alternative to
oversampling is interpolation, which may be used to obtain signal values for
positions that do not coincide with grid nodes, as in [@southern_spatial_2012].
However, the interpolated approach given in this paper is only applicable in
the 2D case, so it cannot be applied in Wayverb, which requires a general
solution in three dimensions.

Finally, this method places an additional constraint on the portion of the
spectrum of the output signal which may be considered valid. The maximum valid
frequency of the output spectrum is determined by the ratio between the
receiver spacing and the wavelength at that frequency. To raise the upper
frequency limit, the receiver spacings must be made as small as possible, which
requires costly mesh oversampling. Small receiver spacings reduce the
sensitivity at the lower end of the spectrum (even when interpolation is used),
so ideally the simulation must use several sets of receiver nodes with
different spacings, in order to maintain a flat frequency response.

This technique seems suitable for calculating lower-order ambisonic signals in
small modelled spaces.  For higher order outputs, with consistent frequency
responses across the spectrum, the technique quickly becomes too expensive (due
to mesh oversampling) and complex to implement (as several receiver spacings
and configurations are required for a single simulation output).

#### Intensity Calculation Technique

The second method considered for implementation in Wayverb is described by
Hacıhabiboğlu in [@hacihabiboglu_simulation_2010].  This method is based around
estimating the acoustic intensity at the output node of the DWM.

The technique is much more straight-forward than the technique discussed above.
The pressure differences between the "output" node and its surrounding nodes
are calculated. These pressures are transformed depending on the relative
positions of each surrounding node, and combined to produce a three-element
vector containing an approximation for the three-dimensional pressure gradient.
The equations describing these operations in the general case are not
replicated here, however they may be greatly simplified in the rectilinear
case, as will be shown in the [Implementation] section below.  The velocity for
each time step is found by adding the pressure gradient vector to the velocity
at the previous time step, where the initial velocity is assumed to be zero.
Intensity is found by multiplying this velocity by the pressure at the output
node.

In some respects, this technique is very similar to the Blumlein difference
technique. Both techniques calculate the instantaneous pressure gradient at the
output node by subtracting the pressures of the surrounding nodes. The main
difference is that the Blumlein method requires that the mesh pressure is found
at exact locations relative to the output node, which often requires mesh
oversampling or interpolation. The intensity calculation technique instead uses
a more general matrix-based method, which incorporates the relative node
positions into the calculation. In short, the Blumlein method requires that the
mesh is designed to suit the receiver configuration, whereas the intensity
calculation method can adapt to arbitrary mesh topologies and spacings.

The frequency range issues mentioned in [@southern_methods_2007] (where large
mesh spacings reduce the output bandwidth, and low mesh spacings reduce low
frequency sensitivity) are mentioned by Hacıhabiboğlu in
[@hacihabiboglu_simulation_2010].  It is unclear whether the intensity
calculation technique solves this problem, although the results presented in
[@hacihabiboglu_simulation_2010] appear consistent across the spectrum.  Some
inaccuracy is evident at the top of the spectrum, but this is attributed to
direction-dependent dispersion error, rather than to a problem with the
frequency response of the model itself.

Under this scheme, the output of the waveguide mesh is actually two streams of
information: a stream of scalar values describing the pressure at the output
node; and a stream of 3D vectors denoting the directional intensity at that
point.  

To find the directional response for a single time-step, the intensity vector
is used to look up the microphone response in the direction of the vector,
which is a scalar value. The microphone response is squared, and multiplied by
the magnitude of the intensity vector, yielding a positive scalar output.
Finally, the sign of the actual pressure at the output node is copied to the
output value. To find the full directional response, this process is repeated
for every time-step of the simulation.

The intensity calculation technique can also model receivers with
direction-dependent frequency responses.  The intensity and pressure outputs
are processed with a set of polar patterns corresponding to responses at
different frequency bands. Then, these processed signals are band-passed with
linear-phase filters, and summed together.  

#### Choice of Method

The intensity calculation technique was chosen for implementation in Wayverb,
as it is simpler to implement, and less expensive to run.  Also, the Blumlein
difference technique is only practical in rectilinear meshes, whereas the
intensity calculation technique can be adapted to any mesh topology. This was
an important consideration at the beginning of the Wayverb project, which
originally used the tetrahedral mesh due to its relative efficiency.

### Implementation 

The implementation is very simple in the rectilinear mesh. The mesh is
inherently aligned to the axial directions, which means that the matrix maths
is greatly simplified. The process for finding the instantaneous intensity
vector for each step of the waveguide simulation is shown in pseudocode below.

~~~ {.python}
def compute_intensities_at_node(node_position,
								mesh_spacing,
								mesh_sample_rate,
								ambient_density):
	velocity_vector = (0, 0, 0)
	pressures = []
	intensities = []

	(nx_position, px_position,
	 ny_position, py_position,
	 nz_position, pz_position) = compute_neighbouring_positions(node_position)

	for step in waveguide:
		//	Find current pressure at output node.
		p = get_node_pressure(node_position)

		//	Calculate the pressure differences between surrounding nodes.
		nx_difference = (get_node_pressure(nx_position) - p) / mesh_spacing
		px_difference = (get_node_pressure(px_position) - p) / mesh_spacing
		ny_difference = (get_node_pressure(ny_position) - p) / mesh_spacing
		py_difference = (get_node_pressure(py_position) - p) / mesh_spacing
		nz_difference = (get_node_pressure(nz_position) - p) / mesh_spacing
		pz_difference = (get_node_pressure(pz_position) - p) / mesh_spacing

		//	Find the instantaneous pressure gradient.
		pressure_gradient_vector = ((px_difference - nx_difference) / 2,
							        (py_difference - ny_difference) / 2,
							        (pz_difference - nz_difference) / 2)

		//	Update the velocity accumulator.
		velocity_vector -= (pressure_gradient_vector /
							(ambient_density * mesh_sample_rate))

		//	Find the instantaneous intensity.
		intensity_vector = velocity_vector * p

		pressures.append(p)
		intensities.append(intensity_vector)

	return pressures, intensities
~~~

For each waveguide step, the pressure and intensity at the output node is found
and stored. If there are multiple coincident capsules, then the same
pressure-intensity data can be post-processed once for each virtual capsule,
without needing to re-run the simulation. However, this approach may not be
well-justified: according to Hacihabiboglu et al. this microphone model is only
suitable for single-diaphragm directional microphones, and is unsuitable for
coincident pressure gradient microphones with multiple elements
[@hacihabiboglu_simulation_2010]. Despite its unsuitability, there is no other
possible way of modelling coincident capsules using this microphone model, so
coincident capsules *are* allowed in Wayverb. Development of a microphone
model with better support of coincident capsules is a topic for future
research.

Having generated the pressures and the intensity vectors at the output node,
the post-processing progresses by finding the magnitude of the intensity
vector.  This value is multiplied by the square of the polar-pattern
attenuation in the direction of the intensity vector, and then the square root
is taken.  Finally, the sign of the pressure is copied to this output value.
This process is repeated for each step of the simulation, resulting in a
sequence of directionally-processed pressure values.

For binaural results, this process is repeated once for each frequency band,
resulting in several differently-weighted broadband signals.  The directional
gains are found using the same lookup-table technique as the geometric methods,
using the azimuth and elevation of the intensity vector as indices. The final
output is created by bandpass filtering each of these broadband signals using a
zero-phase frequency-domain filter, and then mixing them down. The filtering
process is the same as that used throughout Wayverb (see [Ray Tracer]({{
site.baseurl }}{% link ray_tracer.md %})).

### Testing of the Microphone Model in the DWM

The testing procedure is similar to that described in
[@hacihabiboglu_simulation_2010].  A receiver is placed at the centre of a room
measuring $1.5 \times 1.5 \times 1.5$ metres.  Sixteen equally-spaced source
positions are generated in a circle with radius 1m around the receiver.  For
each source, the room is simulated using a rectilinear waveguide mesh with a
sampling rate of 50kHz, for 294 steps.  The step number is chosen so that the
initial wave-front will reach the receiver, but reflections from the room walls
will not.  After each simulation, the directionally-weighted signal recorded at
the receiver is split into frequency bands, and the normalised energy per band
is calculated. The bands are split in the frequency domain, using the
frequency-domain envelope method mentioned in the [Ray Tracer]({{ site.baseurl
}}{% link ray_tracer.md %}) section. The normalised energy of the band is
calculated by dividing the sum of squared bin magnitudes in the output by the
integrated frequency-domain envelope.

The per-band normalised energy can now be plotted against the angle of
incidence, and compared to the ideal polar pattern of the modelled microphone.
This is achieved by normalising the measured directional response relative to
the response in the front direction.  If there is a close match, then the model
is successful.

Three different microphone polar patterns are simulated: omnidirectional,
bidirectional, and cardioid.  The results are shown in +@fig:omnidirectional to
!@fig:bidirectional. In these figures, the red lines show the measured energy
in each direction, while the blue lines show the expected polar pattern,
normalized so that the 0-degree level matches the experimentally-obtained level
in that direction.

![The directional response of an omnidirectional receiver in the waveguide
mesh.](images/Omnidirectional_response){#fig:omnidirectional}

![The directional response of a cardioid receiver in the waveguide
mesh.](images/Cardioid_response){#fig:cardioid}

![The directional response of a bidirectional receiver in the waveguide
mesh.](images/Bidirectional_response){#fig:bidirectional}

Results are consistent across all polar patterns tested.  At the lower
frequencies shown, the match is particularly good, with virtually no error in
all directions.  As frequency increases, the error increases greatly. This
happens as a result of frequency-dependent dispersion. At higher frequencies,
the speed of wave-fronts changes depending on their direction. In the
rectilinear mesh, there is no dispersion along diagonal directions
[@kowalczyk_room_2011]. Along axial directions, however, high-frequency
wave-fronts move more slowly. This is likely the cause of the predominant
"spikes" seen in the diagonal directions: in these directions, all the acoustic
energy reaches the receiver, while along axial directions, the wave-fronts are
slower, and the full energy does not reach the receiver within the simulation
window.

These results mirror those seen by Hacıhabiboğlu, who also records similar
large errors in diagonal directions in the highest (8kHz) band. These errors
are also attributed to directional dispersion.  Therefore, error in the results
is likely due to the properties of the waveguide and the particular receiver
technique used, rather than an error in the implementation.

Although there is very large error in some directions at high frequencies, at
low frequencies this method produces accurate directional responses.  Even at
higher frequencies (relative to the mesh sampling rate), the response follows
the same overall shape as the desired response, so even here the results may
provide a reasonable approximation of the directional room response.  Finally,
if accurate directional responses are required at higher frequencies, the mesh
can be oversampled to reduce directional dispersion. Although costly, it is at
least potentially possible to produce very accurate responses using this
method.
