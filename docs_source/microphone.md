---
layout: page
title: Microphone modelling
navigation_weight: 6
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
these directional cues must be recorded and reproduced.  Single omnidirectional
microphones cannot record directional responses, and so a different
direction-sensitive receiver model must be developed and used instead.

## Background

Humans are able to detect the originating direction of sounds using two main
techniques: Interaural Time Difference (ITD) defines the time delay between
ears, when the sound source is located nearer to one ear than the other; and
Interaural Level Difference (ILD) denotes the difference in sound level (which
may vary by frequency) caused by attenuation by the outer ear, head, and torso.
The success of virtual acoustics depends on the ability to produce signals with
appropriate ITD and ILD, in such a way that the sound is perceived as coming
from a certain direction.

When recording impulse responses of physical spaces, several techniques might
be used to retain ITD and ILD information.

One option is to record the impulse response using a matched pair of
microphones.  A spaced pair of omnidirectional microphones (known as an AB
pair) will capture interchannel time difference, but will not capture
interchannel level difference if the source is positioned far away from the
microphones. Alternatively, a coincident pair of cardioid capsules (an XY pair)
or bidirectional capsules (a Blumlein pair) might be used. These configurations
capture level difference, but are incapable of recording time difference,
because wave-fronts will always arrive at both capsules at the same time.

Microphone pair methods are only suitable for recording stereo signals, as they
only capture a two-dimensional "slice" through the modelled scene, where all
directional information is restricted to the same plane as the microphone
capsules. The technique can be extended to higher dimensions by using more
microphone capsules. This is the basis of the ambisonic technique, which uses
four coincident directional microphone capsules to capture the
three-dimensional directional pressure gradient. Instead of being used
directly, the recorded signals are post-processed depending on the
configuration of the output speakers. A three dimensional speaker-array can
perfectly reproduce the sound field captured by the microphones. For playback
on headphones, the signals can be filtered with *head related transfer
functions* (HRTFs), which modify the frequency content of the sound depending
on its originating direction, mimicking the absorptive characteristics of the
human head and torso [@noisternig_3d_2003]. As the microphone capsules are all
coincident, ITD cannot be perfectly reconstructed (the ITD differs depending on
the direction and distance of the source, and ambisonic techniques cannot
record distance, only the pressure gradient direction). Therefore, ambisonics
is not the optimal choice for headphone playback.

The preferred method for capturing impulse responses which retain ILD *and* ITD
in a format suitable for headphone playback is the "dummy head" microphone
pair. This is a model of a human torso, constructed from materials which mimic
the acoustic properties of a real torso, with a microphone in each ear.
Recordings made with a dummy head capture the ITD and ILD that would be
perceived by a real human listener, and produce a convincing reconstruction of
the sonic space when played back over headphones. However, responses recorded
using this technique are unsuitable for loudspeaker playback.

All of these techniques require that the receiver gain is dependent upon the
incident direction. The dummy head technique additionally requires that the
frequency response of the receiver is dependent upon the incident direction.
To allow reproduction on both headphones and arbitrary loudspeaker arrays, the
receiver model in Wayverb should encompass all of the techniques described
above. Given that spaced microphone techniques can be modelled simply by
including multiple receivers in the simulation, each virtual capsule should
have configurable direction-dependent gain and frequency response, allowing it
to behave like a standard microphone, or like the ear of a dummy head.

## Calculating Directional Gain

Wayverb contains two different models for calculating an appropriate gain from
an incident direction, described here.

### Microphone

The first method, the "perfect microphone", is very simple.  Given that the
virtual microphone is pointing in direction $\hat{p}$, and has a "shape"
parameter $0 \leq s \leq 1$, the direction-dependent attenuation $a(\hat{d})$
from direction $\hat{d}$ is given by

(@) $$a(\hat{d}) = (1 - s) + s(\hat{d} \cdot \hat{p})$$

where $\hat{p}$ and $\hat{d}$ are unit vectors, and $\cdot$ is the dot-product
operator. When $s$ is set to 0, the $a(\hat{d})$ is equal to one for all values
of $\hat{d}$, modelling an omnidirectional polar pattern. When $s$ is 1, the
modelled polar pattern is bidirectional, with a gain of 1 in front the
microphone, 0 at either side, and -1 behind. When $s$ is 0.5, the pattern is
cardioid. Sub- and supercardioid patterns are created by setting $s$ lower or
higher than 0.5 respectively. This microphone model is flexible, allowing
almost all standard microphone techniques (including the XY pair, Blumlein
pair, and B-format ambisonic) to be simulated.

Note that the frequency response of this model is independent of direction, so
it can only be used to model ideal microphones. It is not suitable for
modelling physical microphones with direction-dependent frequency responses. In
particular, with this technique it is not possible to produce binaural
dummy-head-style recordings, for immersive headphone playback.

### HRTF

A more general microphone model would allow specific per-frequency-band gains
to be set for each incoming direction. However, this general model is *only*
really useful for simulating the direction-dependent frequency response of
dummy head microphones.  For this reason, the general microphone model in
Wayverb is restricted to the modelling of HRTF characteristics.

The method itself is based on a two-dimensional lookup table, indexed by the
azimuth and elevation angles formed by the incident direction. The table is
produced using experimentally-obtained HRTF data from the IRCAM Listen database
[@oliver_listen_2003]. The data takes the form of a collection of stereo audio
files, where each audio file contains the impulse responses measured at the
right and left ears in an anechoic environment, for an impulse originating at a
specific azimuth and elevation angle. These audio files are
multi-band-filtered, using the same 8-band frequencies used throughout Wayverb,
and the per-band per-channel energy is recorded to a lookup table.

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
recording signals for each of 288 incident directions, which might easily
occupy more than a gigabyte of memory (288 3-second impulse responses at
32-bits and 44100KHz require 1160MB of memory), and then separately convolving
and mixing them. This is definitely possible, but is inefficient considering
that the rest of the simulation only runs with eight-band resolution. Even if
the directional processing is extremely precise, it can't reconstruct or "make
up for" the relatively low precision of the simulation frequency response.
Therefore, it is not unreasonable to approximate the HRTF frequency responses,
and take advantage of the resulting efficiency gains.

## Image Source Implementation

As described in the [Image Source]({{ site.baseurl }}{% link image_source.md
%}) section, the magnitude of the contribution of each image source depends on
the distance between the source and receiver, and the direction-dependent
reflectance characteristics of any intermediate reflective surfaces. To model a
directional receiver, the contribution magnitude is additionally scaled by the
receiver's gain in the direction of that contribution.

It is very easy to obtain the direction of each image source: simply subtract
the receiver position from the image source position, and normalise the result.
This direction can either be used to directly compute the appropriate
attenuation factor, or it can be used to look up the attenuation in a
pre-computed table (see [Calculating Directional Gain] below).

It is also easy to adapt this technique, in the case that the frequency
response of the receiver depends on incident direction, rather than just the
overall gain.  In Wayverb, image-source contributions are calculated in eight
frequency bands, which are filtered and mixed down at the end of the
simulation.  A direction-dependent frequency response can be modelled simply by
using a different attenuation function or look-up table for each frequency
band.  Using this method, interchannel level difference can be modelled with
accuracy matching the rest of the image-source simulation.

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

<!-- TODO mention interchannel time difference method for image source? -->

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
ray_tracer.md %}}) section. This achieved by taking the centre direction of
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
direction of the sound field must be found at the receiver point.  This might
be achieved through several methods, which are discussed below.

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
full first-order B-format recording can be made using seven such "pressure
sensors", arranged in a cross shape, with a single node at the centre, and six
nodes placed around it.  The pressure field velocity components are found by
subtracting nodes on "opposite" sides of the cross, while the centre node
contributes the overall pressure at the receiver. This receiver configuration
produces a first-order B-format ambisonic recording. A second-order recording
is achieved by adding further pressure-sensing nodes in between the first-order
nodes.

Note that this technique does not allow the immediate production of directional
responses. Instead, it is designed to produce recordings in ambisonic format,
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

It is suggested to increase the spatial sampling frequency of the mesh in order
to facilitate more exact placement of sensor nodes (by reducing the grid
spacing, the quantisation error in the sensor placement is also reduced).  The
example given in [@southern_2nd_2007] uses a sensor spacing of 0.04 metres,
giving a maximum valid sampling frequency of 4.25kHz.  However, to allow
accurate placement of these sensors, a grid with a spacing of 0.0027m is
required.  Such a grid will need more than 3000 times the original number of
nodes in order to fill the same volume, which is only practical for very small
modelled spaces.

Finally, this method places an additional constraint on the portion of the
spectrum of the output signal which may be considered valid. The upper limit of
the output spectrum is determined by the ratio between the receiver spacing and
the wavelength frequency. To raise the upper frequency limit, the receiver
spacings must be made as small as possible, which requires costly mesh
oversampling. Small receiver spacings reduce the sensitivity at the lower end
of the spectrum, so ideally the simulation will use several sets of receiver
nodes with different spacings, in order to maintain a flat frequency response.

This technique seems suitable for calculating lower-order ambisonic signals in
small modelled spaces.  For higher order outputs, with flatter frequency
responses, the technique quickly becomes too expensive (due to mesh
oversampling) and complex to implement (as several receiver spacings and
configurations are required for a single recording).

#### Intensity Calculation Technique

The second method is described by Hacıhabiboglu in
[@hacihabiboglu_simulation_2010].  This method is based around estimating the
acoustic intensity at the output node of the DWM.

The technique is much more straight-forward than the technique discussed above.
The pressure differences between the "output" node and its surrounding nodes
are calculated and placed into a column vector.  This vector is multiplied by
the inverse of a matrix containing the relative positions of the surrounding
nodes to the output node. The result of this calculation is a three-element
vector containing an approximation for the derivative of pressure with respect
to each of the three axial directions.  The velocity for each time step is
found by adding the pressure-derivative vector to the velocity at the previous
time step, where the initial velocity is assumed to be zero.  Intensity is
found by multiplying this velocity by the pressure at the output node.

In some respects, this technique is very similar to the Blumlein difference
technique. Both techniques calculate the instantaneous pressure gradient at the
output node by subtracting the pressures of the surrounding nodes. The main
difference is that the Blumlein method requires that the mesh pressure is found
at exact locations relative to the output node, which often requires mesh
oversampling. The intensity calculation technique instead uses a more general
matrix-based method, which incorporates the relative node positions into the
calculation. In short, the Blumlein method requires that the mesh is designed
to suit the receiver configuration, whereas the intensity calculation method
can adapt to arbitrary mesh topologies and spacings.

The frequency range issues mentioned in [@southern_methods_2007] (where large
mesh spacings reduce the output bandwidth, and low mesh spacings reduce low
frequency sensitivity) are mentioned by Hacıhabiboglu in
[@hacihabiboglu_simulation_2010].  It is unclear whether the intensity
calculation technique solves this problem, although the results presented in
that paper appear consistent across the spectrum.  Some inaccuracy is evident
at the top of the spectrum, but this is attributed to direction-dependent
dispersion error, rather than to a problem with the frequency response of the
model itself.

Under this scheme, the output of the waveguide mesh is actually two streams of
information: a stream of scalar values describing the pressure at the output
node; and a stream of 3D vectors denoting the directional intensity at that
point.  A directional response can be found by multiplying the magnitude of the
intensity vector by the squared magnitude of a microphone polar pattern in the
direction of the intensity, for each time-step.  The sign of the directional
response should be restored from the pressure.

The intensity calculation technique can also model receivers with
direction-dependent frequency responses.  The intensity and pressure outputs
are processed with a set of polar patterns corresponding to responses at
different frequency bands. Then, these processed signals are band-passed with
linear-phase filters, and summed together.  

#### Choice of Method

It appears that the intensity calculation technique is superior for arbitrary
receiver emulation in the DWM.  The Blumlein difference technique will be much
more expensive because it requires mesh oversampling.

If a B-format ambisonic output is required, the intensity calculation technique
can be used to model receivers with the particular polar patterns required.
The intensity calculation technique appears capable of everything that the
Blumlein difference technique can do, but in a more general form.

Finally, the intensity calculation technique can be adapted to any mesh
topology. This was an important consideration at the beginning of the Wayverb
project, which originally used the tetrahedral mesh due to its relative
efficiency. The Blumlein difference technique could not be used because of the
difficulty of applying it to non-rectilinear meshes, and the intensity
calculation method was the only remaining choice.

Even in rectilinear meshes, the intensity calculation technique appears to have
more desirable characteristics, and so it was chosen for use in Wayverb.

### Implementation 

The implementation is very simple in the rectilinear mesh. The mesh is
inherently aligned to the axial directions, which means that the matrix maths
is greatly simplified. The process for finding the instantaneous intensity for
each step of the waveguide simulation is shown in pseudocode below.

~~~ {.python}
def compute_intensities_at_node(node_position,
								mesh_spacing,
								mesh_sample_rate,
								ambient_density):
	velocity = (0, 0, 0)
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
		pressure_gradient = ((px_difference - nx_difference) / 2,
							 (py_difference - ny_difference) / 2,
							 (pz_difference - nz_difference) / 2)

		//	Update the velocity accumulator.
		velocity -= (pressure_gradient / (ambient_density * mesh_sample_rate))

		//	Find the instantaneous intensity.
		intensity = velocity * p

		pressures.append(p)
		intensities.append(intensity)

	return pressures, intensities
~~~

For each waveguide step, the pressure and intensity at the output node is found
and stored.  If there are multiple coincident capsules, then the same
pressure-intensity data can be post-processed once for each virtual capsule,
without needing to re-run the simulation.

The post-processing step is also very simple.
For each pressure-intensity pair, the magnitude of the intensity is found.
This value is multiplied by the square of the polar-pattern attenuation in the
direction of the intensity vector, and then the square root is taken.
Finally, the sign of the pressure is copied to this output value.

For binaural results, this process is repeated several times, once for each
band, resulting in eight differently-weighted broadband signals.  The
directional gains are found using the same lookup-table technique as the
geometric methods, using the azimuth and elevation of the intensity vector as
indices. The final output is created by bandpass filtering each of these
broadband signals using a zero-phase frequency-domain filter, and then mixing
them down.

Note that, for binaural signals found using the same output node for both ears,
interaural time difference will not be taken into account.  For high-accuracy
simulations, where ITD is required, two receiver nodes should be used, one at
each ear position.  Of course, the grid spacing must then be fine enough to
position the ear nodes at the correct distance from one another.

### Testing Procedure

The testing procedure is similar to that described in
[@hacihabiboglu_simulation_2010].  A receiver is placed at the centre of a room
measuring $1.5 \times 1.5 \times 1.5$ metres.  Sixteen equally-spaced source
positions are generated in a circle with radius 1m around the receiver.  For
each source, the room is simulated using a rectilinear waveguide mesh with a
sampling rate of 50KHz, for 294 steps.  The step number is chosen so that the
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
bidirectional, and cardioid.  The results are shown in the following
figures\text{ (\ref{fig:omnidirectional} to \ref{fig:bidirectional})}. 

![The directional response of an omnidirectional receiver in the waveguide mesh.\label{fig:omnidirectional}](images/Omnidirectional_response)

![The directional response of a cardioid receiver in the waveguide mesh.\label{fig:cardioid}](images/Cardioid_response)

![The directional response of a bidirectional receiver in the waveguide mesh.\label{fig:bidirectional}](images/Bidirectional_response)

Results are consistent across all polar patterns tested.  At low frequencies,
the match is particularly good, with virtually no error in all directions.  As
frequency increases, the error increases greatly. This happens as a result of
frequency-dependent dispersion. At higher frequencies, the speed of wave-fronts
changes depending on their direction. In the rectilinear mesh, there is no
dispersion along diagonal directions [@kowalczyk_room_2011]. Along axial
directions, however, high-frequency wave-fronts move more slowly. This is
likely the cause of the predominant "spikes" seen in the diagonal directions:
in these directions, all the acoustic energy reaches the receiver, while along
axial directions, the wave-fronts are slower, and the full energy does not
reach the receiver within the simulation window.

These results mirror those seen by Hacıhabiboglu, who also records similar
large errors in diagonal directions in the highest (8KHz) band. These errors
are also attributed to directional dispersion.  Therefore, error in the results
is likely due to the properties of the waveguide and the particular receiver
technique used, rather than an error in the implementation.

### Conclusion

Although there is very large error in some directions at high frequencies, at
low frequencies this method produces extremely accurate directional responses.
Even at high frequencies, the response follows the same overall shape as the
desired response, so even high-frequency results may provide a reasonable
approximation of the directional room response. Finally, if accurate
directional responses are required at high frequencies, the mesh can be
oversampled to reduce directional dispersion. Although costly, it is at least
potentially possible to produce very accurate responses using this method.
