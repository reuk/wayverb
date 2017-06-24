---
layout: page
title: Evaluation
navigation_weight: 24 
---

---
reference-section-title: References
...

# Evaluation {.major}

This section describes the Wayverb program, and demonstrates some example
simulation results.  The simulations are chosen to highlight the behaviour of
the simulator with respect to parameters such as reverb time, frequency
content, and early reflection times. The project files for each of these tests
is included in the Wayverb distribution.

## Features

The Wayverb program has the following features:

- **Hybrid Geometric and Waveguide Simulation**: This is the most important
  feature of Wayverb, providing the ability to simulate the acoustics of
  arbitrary enclosed spaces.
- **Load Arbitrary Models**: The model-importing functionality is built on top
  of the Assimp library, which has support for a wide variety of 3D formats
  [@_assimp_2017].  Care must be taken to export models with the correct scale,
  as Wayverb interprets model units as metres. The detected model dimensions are
  shown in the interface, so that model dimensions can be checked, and the model
  can be re-exported if necessary.
- **Visualiser**: Allows the state of the simulation to be observed, as it
  changes.
- **Unlimited Sources and Receivers**: Set up any number of sources and
  receivers. This has the trade-off that the simulation will automatically run
  once for each source-receiver pair, which will be time consuming when there are
  many combinations.
- **Multiple Capsules per Receiver**: Each receiver behaves like a set of
  coincident capsules.  Each capsule may model an ideal microphone, or an HRTF
  ear. Multiple capsules at the same receiver require only a single simulation
  run, so multi-capsule receivers should be preferred over multiple receivers,
  wherever possible. For HRTF simulations, the receiver position will be
  automatically adjusted during the image-source simulation, replicating the
  interaural spacing of a real pair of ears (see the Image Source
  Implementation subsection of the [Microphone Modelling]({{ site.baseurl }}{%
  link microphone.md %}) section). This produces a realistic stereo time-delay
  effect in the early-reflection portion of the output, aiding localisation.
- **Custom Materials**: Wayverb reads unique material names from the loaded 3D
  model.  Each unique material in the model may be assigned custom acoustic
  properties, consisting of multi-band absorption and scattering coefficients.
- **Tunable Ray Tracer**: The number of rays is controlled by a quality
  parameter, which defines the number of rays which are expected to intersect the
  receiver per histogram interval. Higher quality values will lead to more
  accurate reverb tails, at the cost of longer processing times. The desired
  image-source depth can also be varied from 0 to 10, although lower values
  are recommended. In the real world, the ratio of scattered to non-scattered
  sound energy will increase as the impulse response progresses. The
  image-source model does not account for scattering. Therefore, lower
  image-source reflection depths are more physically plausible, as the
  simulation will switch to stochastic ray-tracing (which *does* account for 
  scattering) sooner.
- **Tunable Waveguide**: The waveguide has two modes: a single-band mode which
  uses the Yule-Walker method to estimate boundary filter parameters, and a
  multi-band mode which uses "flat" filters. These filters are able to model
  a given wall absorption with greater accuracy, but only when the wall 
  absorption is constant across the spectrum. The multi-band mode is therefore
  significantly slower, as it must run the waveguide process several times. It
  uses the wall absorption from each frequency band in turn, and then band-pass
  filters and mixes the results of each simulation to find the final output.
  Both waveguide modes allow the maximum waveguide frequency, and the oversampling
  factor, to be modified.

The interface of the Wayverb program is explained in +@fig:wayverb_ui.

![The interface of the Wayverb program.](images/wayverb_ui){#fig:wayverb_ui}

## Tests

Some aspects of the Wayverb algorithm have already been tested in previous
sections, and so do not require further testing here.

Specifically, the [Hybrid Model]({{ site.baseurl }}{% link hybrid.md %})
section compares the waveguide to an ideal image-source model, showing that the
output level is correctly matched between models. This test also shows that the
modal response of the waveguide matches the "ideal" response for several
different values of absorption coefficients, implying that the waveguide and
image-source boundary models are consistent.

The [Microphone Modelling]({{ site.baseurl }}{% link microphone.md %}) section
shows that the waveguide model is capable of simulating directionally-dependent
receivers, with gain dependent on the angle of the incident wave-front.

Finally, the [Boundary Modelling]({{ site.baseurl }}{% link boundary.md %})
section shows that the waveguide boundaries exhibit the expected wall impedance
(though with some error, which increases at higher frequencies).

In the tests below, all impulse responses are produced using the Wayverb
software.  Reverb times are calculated using the Room EQ Wizard [@_room_2017].
The test projects can be found in the Wayverb repository.

<div id="audio_table">

The following file is used as a carrier signal for testing the generated
impulse responses. It starts with a single Dirac impulse, which will produce a
single copy of the generated impulse response in the output. This is followed
by a drum loop, a piano phrase, a guitar melody, and a short string sample, all
taken from the Logic Pro loop library. The final sample is an operatic voice,
taken from the OpenAir anechoic sound database [@_operatic_2017] and reproduced
under the terms of the Creative Commons BY-SA license.  These sounds are chosen
as they have no reverb applied, and so will give an accurate representation of
the applied reverbs. Additionally, they cover a wide frequency range, and
contain both sustained and transient material, presenting the reverbs under a
range of conditions.

--------------------------------------------------------------------------------
name                            audio file
------------------------------- ------------------------------------------------
carrier signal                  <audio controls><source src="demos/out_cardioid_away.mp3" type="audio/mpeg">This browser does not support html5 audio.</audio>
--------------------------------------------------------------------------------

</div>

### Terms and Measurements

Here, reverb times are described using the *RT60*, which is a parameter
denoting the time taken for the sound level in a space to fall by sixty
decibels [@_reverberation_2017]. Commonly, impulse response recordings do not
have the necessary dynamic range of 75dB or more which is necessary to directly
calculate the RT60. Instead, a smaller level decrease is timed, and this time
is extrapolated to give an estimate of the RT60. The time taken for a twenty or
thirty decibel decrease is used, and multiplied by two or three respectively to
estimate the RT60. These measurements are known as the T20 and T30. 

The estimated or predicted RT60, $T$, of a given space can be calculated using
the Sabine formula [@kuttruff_room_2009, p. 131]:

$$T=0.161\frac{V}{A}$$ {#eq:}

where $V$ is the volume of the space in cubic metres, and $A$ is the
*equivalent absorption area*.  The absorption area of a given surface is equal
to the area of the surface multiplied by its absorption coefficient. The
equivalent absorption area for an entire room is found by summing the
absorption areas of all surfaces in the scene.

### Reverb Times for Varying Room Volumes

This test aims to check that rooms with different volumes produce the expected
reverb times. Rooms with different volumes, but the same absorption
coefficients and source/receiver positions, are simulated. Then, the RT60 is
calculated from the simulated impulse responses, and compared against the
Sabine estimate. A close match shows that the change in room volume has the
correct, physically plausible effect on the generated outputs.

Three different cuboid rooms with the following dimensions are modelled:

- **small**: $2 \times 2.5 \times 3$ metres
- **medium**: $4.5 \times 2.5 \times 3.5$ metres
- **large**: $12 \times 4 \times 8$ metres

Each room is set to have absorption and scattering coefficients of 0.1 in all
bands. The source and receiver are placed 1 metre apart at the centre of each
room.  The waveguide is used to model frequencies up to 500Hz, using a mesh
with a sampling rate of 3330Hz.  The image-source model generates reflections
up to the fourth order.

The following results are found, for the entire (broadband) output:

--------------------------------------------------------------------------------
room                Sabine RT / s   measured T20 / s   	measured T30 / s
------------------- --------------- ------------------- ------------------------
small               0.653           0.663               0.658

medium              0.887           0.897               0.903

large               1.76            1.86                1.96
--------------------------------------------------------------------------------

The results for small and medium rooms are within 5% of the expected reverb
time, although the measured T30 of the larger room has an error of 11%.
Increasing the room volume has the effect of increasing the reverb time, as
expected.

Now, the results are plotted in octave bands (see +@fig:room_size_rt30).  The
results in lower bands, which are modelled by the waveguide, have a
significantly shorter reverb time than the upper bands, which are generated
geometrically. The higher bands have reverb times slightly higher than the
Sabine prediction, while the waveguide-generated bands show much shorter reverb
times tails than expected. The difference in reverb times between the waveguide
and geometric methods also becomes evident when spectrograms are taken of the
impulse responses (see +@fig:room_size_spectrograms). In all tests, the initial
level is constant across the spectrum, but dies away faster at lower
frequencies. In the medium and large rooms, some resonance at 400Hz is seen
towards the end of the reverb tail, which is probably caused by numerical
dispersion in the waveguide mesh.

In the medium and large tests, the spectrograms appear as though the
low-frequency portion has a longer, rather than a shorter, reverb time.
However, in the large test, the late low-frequency energy has a maximum of
around -100dB, which is 40dB below the level of the initial contribution. The
measured T20 and T30 values do not take this into account, and instead reflect
the fact that the *initial* reverb decay is faster at low frequencies. The
spectrograms show that the waveguide sometimes resonates for an extended period
at low amplitudes. In effect, the waveguide exhibits a high noise-floor under
some conditions.

![T30 in octave bands, calculated from the measured impulse
responses.](images/room_size_rt30){#fig:room_size_rt30}

![Spectrograms of impulse responses obtained from different room
sizes.](images/room_size_spectrograms){#fig:room_size_spectrograms}

This result is difficult to explain. A shorter reverb time indicates that
energy is removed from the model at a greater rate than expected. Energy in the
waveguide model is lost only at boundaries, so the most likely explanation is
that these boundaries are too absorbent. It is also possible that the
microphone model causes additional unexpected attenuation.

Further tests (not shown) of the three rooms were carried out to check possible
causes of error.  In one test, the Yule-Walker-generated boundary filters were
placed with filters representing a constant real-valued impedance across the
spectrum, to check whether the boundary filters had been generated incorrectly.
In a second test, the modelled omnidirectional microphone at the receiver was
removed, and the raw pressure value at the output node was used instead, to
check that the microphone was not introducing undesired additional attenuation.
However, in both tests, similar results were produced, with reverb times
significantly lower than the Sabine prediction. The boundary and microphone
models do not appear to be the cause of the problem.

The reverb-time test at the end of the [Hybrid Model]({{ site.baseurl }}{% link
hybrid.md %}) section shows that the waveguide reverb times match the reverb
times of the exact image-source model, which will be close to the analytical
solution in a cuboid room. The close match to the almost-exact image-source
model suggests that the waveguide and boundary model have been implemented
correctly. Additionally, the tests in the [Boundary Modelling]({{ site.baseurl
}}{% link boundary.md %}) section show that wall impedances are accurately
modelled.

Given that in all previous tests the waveguide behaves as expected, one
possibility is that the Sabine equation is simply a poor predictor of
low-frequency reverb times, or reverb times in regularly-shaped rooms with
well-defined reflection patterns (such as cuboids). If this were the case, this
might justify the waveguide results. This is a reasonable suggestion: the
Sabine equation assumes that the sound field is diffuse, which in turn requires
that at any position within the room, reverberant sound has equal intensity in
all directions, and random phase relations [@hodgson_when_1994]. This is
obviously untrue in a cuboid at low frequencies, where the non-random phase of
reflected waves causes strong modal behaviour due to waves resonating between
the parallel walls of the enclosure.

Further testing is required to locate the exact cause of the differences in
reverb times. In the present implementation, the mismatch is an obvious
artefact in the output, which affects the impulse response's suitability for
musical applications.

<div id="audio_table">

--------------------------------------------------------------------------------
test                    audio file
----------------------- --------------------------------------------------------
small                   <audio controls><source src="demos/out_small.mp3" type="audio/mpeg">This browser does not support html5 audio.</audio>

medium                  <audio controls><source src="demos/out_medium.mp3" type="audio/mpeg">This browser does not support html5 audio.</audio>

large                   <audio controls><source src="demos/out_large.mp3" type="audio/mpeg">This browser does not support html5 audio.</audio>
--------------------------------------------------------------------------------

</div>

### Reverb Times for Varying Absorptions

This test simulates the same room with several different absorption
coefficients.  The "medium" room from the above test is simulated, again with
the source and receiver placed 1 metre apart in the centre of the room.
Scattering is set to 0.1 in all bands. The absorption coefficients are set to
0.02, 0.04, and 0.08, corresponding to Sabine predictions of 4.43, 2.22, and
1.11 seconds.

--------------------------------------------------------------------------------
absorption          Sabine RT / s   measured T20 / s   	measured T30 / s
------------------- --------------- ------------------- ------------------------
0.02                4.433           4.295               4.283

0.04                2.217           2.210               2.219

0.08                1.108           1.126               1.156
--------------------------------------------------------------------------------

The issue with shorter low-frequency decay times persists in this test.
However, the broadband reverb time responds correctly to the change in
absorption coefficients.

![Spectrograms of impulse responses obtained from simulating the same room with
different absorption coefficients. Note that the low-frequency content has a
shorter decay time than the high-frequency
content.](images/room_material_spectrograms){#fig:room_material_spectrograms}

<div id="audio_table">

--------------------------------------------------------------------------------
test                    audio file
----------------------- --------------------------------------------------------
absorption: 0.02        <audio controls><source src="demos/out_0.02.mp3" type="audio/mpeg">This browser does not support html5 audio.</audio>

absorption: 0.04        <audio controls><source src="demos/out_0.04.mp3" type="audio/mpeg">This browser does not support html5 audio.</audio>

absorption: 0.08        <audio controls><source src="demos/out_0.08.mp3" type="audio/mpeg">This browser does not support html5 audio.</audio>
--------------------------------------------------------------------------------

</div>

### Direct Response Time

The "large" room above is simulated again, but with the source and receiver
positioned in diagonally opposite corners, both 1 metre away from the closest
walls in all directions.  The generated impulse response is compared to the
previous impulse response, in which the source and receiver are placed 1 metre
apart in the centre of the room.

Sound is simulated to travel at 340m/s, so when the source and receiver are
placed 1m apart, a strong impulse is expected after 1/340 = 0.00294 seconds.
In the new simulation, the source and receiver are placed $\sqrt{10^2 + 2^2 +
6^2}$ = 11.8m apart, corresponding to a direct contribution after 0.0348
seconds.

When the source is further away, the direct contribution may not be the loudest
part of the impulse. As the distance from the source $r$ increases, the energy
density of the direct component decreases proportionally to $1/r^2$. However,
the energy density in an ideally-diffuse room is independent of $r$. At a
certain distance, known as the *critical distance* $r_c$, the energy densities
of the direct component and reverberant field will match. Beyond the critical
distance, the energy of the direct component will continue to decrease relative
to the reverberant field [@kuttruff_room_2009, pp. 146-147].

This effect is observed for the larger separating distance. The first and
second order early reflections arrive very shortly after the direct response.
Many of these reflection paths cover the same distance, and so arrive at the
same time. These contributions are added, giving an instantaneous energy which
is often greater than that of the direct contribution. The early reflections
also occur at a greater frequency for the increased spacing. Therefore it is
obvious that the ratio of energy densities between the direct and reverberant
contributions is lower for greater spacings, as expected.

![Larger distances between the source and receiver result in delayed initial
contributions. This figure shows the first 0.05s of the outputs of two
simulations in which the source and receiver are separated by 1m and
11.8m.](images/spacing_signals){#fig:spacing_signals}

<div id="audio_table">

--------------------------------------------------------------------------------
test                    audio file
----------------------- --------------------------------------------------------
near                    <audio controls><source src="demos/out_large.mp3" type="audio/mpeg">This browser does not support html5 audio.</audio>

far                     <audio controls><source src="demos/out_large_spaced.mp3" type="audio/mpeg">This browser does not support html5 audio.</audio>
--------------------------------------------------------------------------------

</div>

### Obstructions

Early reflection behaviour seems to be correct in simple cuboid models, where
there is always line-of-sight between the source and receiver. The behaviour in
more complex models, in which the source and receiver are not directly visible,
must be checked.

The simulated space is a simple vault-like model, similar to a small hall, but
broken up by regularly repeating pillars. The source and receiver are
positioned seven metres apart, with their view obstructed by two pillars. If
there were no obstruction, a strong direct impulse would be expected after
0.0206 seconds. However, the pillars should block this direct contribution.

![The testing set-up, showing the pillars blocking the line-of-sight between
the source and receiver.](images/vault_demo.png)

In the real world, objects with areas of a similar or greater order to the
incident wavelength cause diffraction effects [@kuttruff_room_2009, p. 59].
The result of diffraction is that an additional "diffraction wave" is created
at the edge of the object, which radiates in all directions. In the vault
model, the edges of the pillars should cause diffraction, and in this way, some
energy should be scattered from the source to the receiver. This energy will
arrive slightly after the direct contribution would have, but before the first
early reflection. The shortest possible path from source to receiver which
travels around the pillars has a length of 7.12m, corresponding to a time of
0.0209s. Though the image-source and ray tracing models are not capable of
modelling diffraction effects, the waveguide model inherently models this
phenomenon. Therefore, the impulse response should record a low-frequency
excitation at around 0.0209s.

![The early part of the vault impulse response. Low frequency diffraction from
the waveguide is detected before the first image-source
contribution.](images/vault_response){#fig:vault_response}

The impulse response graph (+@fig:vault_response) shows that low frequency
diffraction is in fact recorded. Though this behaviour is physically correct,
it highlights the main shortcoming of the hybrid algorithm.  The correct
behaviour of the waveguide conflicts with the approximate nature of the
geometric algorithms, causing an obvious divide or disconnect between the low
and high frequency regions in the output. The low frequencies have a fast onset
and immediate decay, whereas the higher frequencies have a delayed onset with
extended decay. This result is physically implausible, and makes the impulse
response unsuitable for realistic, high-quality reverb effects.

<div id="audio_table">

--------------------------------------------------------------------------------
test                    audio file
----------------------- --------------------------------------------------------
vault                   <audio controls><source src="demos/out_vault.mp3" type="audio/mpeg">This browser does not support html5 audio.</audio>
--------------------------------------------------------------------------------

</div>

### Late Reflection Details

Having checked the behaviour of early reflections, now the late-reflection
performance must be checked. The nature of the ray tracing process means that
fine detail (below 1ms precision) is not captured. However, it should be
possible to observe reverb features on a larger scale, such as distinct echoes
from a long tunnel.

A cuboid with dimensions $4 \times 7 \times 100$ metres is simulated. The
receiver is placed exactly at the centre of the model, with the source
positioned two metres away along the z direction.  The output should contain a
direct contribution at 0.00588s, and some early reflections from the nearby
walls. The reverb tail should contain strong echoes every 0.294s, as the
initial wave-front reflects back-and-forth between the two end walls.

The tunnel is modelled using absorption coefficients of 0.03 in the bottom five
bands, then 0.04, and 0.07 in the highest two bands. The scattering
coefficients are set to 0.1 in all bands. This scattering should cause echoes
in the reverb tail to be "smeared" in time.  To check the effect of the
scattering coefficients, the same test is also run using scattering
coefficients of 0 in all bands.

![Spectrograms of the tunnel impulse responses with different scattering
levels.  Note that the first 3 echoes are clear in both responses, but the
scattered response quickly becomes less distinct, while the response with no
scattering has clear echoes which die away more
slowly.](images/tunnel_spectrograms){#fig:tunnel_spectrograms}

The spectrograms show that there are clear increases in recorded energy,
occurring approximately every 0.3 seconds after the initial onset. Each echo is
followed by its own decay tail. In the case of the non-scattering simulation,
the echoes are clearly separated, with very short tails. When the scattering is
increased, the tails become longer, and the individual echoes become less
defined. This is expected: when there is no scattering, rays travelling in any
direction other than along the length of the tunnel will bounce between the
walls many times, attenuated each time. These contributions may arrive at any
time at the receiver, however, their amplitude will be greatly reduced. The
rays travelling directly between the ends of the tunnel will be reflected fewer
times, will lose less energy, and will produce louder regular contributions.
When scattering is introduced, the ray paths are less regular, giving less
correlation between the number of reflections and the time at which the ray is
recorded. This in turn leads to a more even distribution of recorded energy
over time.

<div id="audio_table">

--------------------------------------------------------------------------------
test                            audio file
------------------------------- ------------------------------------------------
tunnel with scattering          <audio controls><source src="demos/out_tunnel.mp3" type="audio/mpeg">This browser does not support html5 audio.</audio>

tunnel without scattering       <audio controls><source src="demos/out_tunnel_no_scatter.mp3" type="audio/mpeg">This browser does not support html5 audio.</audio>
--------------------------------------------------------------------------------

</div>

### Directional Contributions

To test that microphone modelling behaves as expected, a cardioid microphone is
placed in the exact centre of the "large" cuboid room, facing away from the
source, which is positioned at a distance of 3m along the z-axis.

An ideal cardioid microphone has unity gain in its forward direction, and
completely rejects waves incident from behind. In this test, the direct
contribution from the source originates directly behind the receiver, so it
should not be present in the output. The first contribution detected by the
receiver is caused by reflections from the floor and ceiling. Both of these
reflection paths have lengths of exactly 5m, corresponding to 0.0147s. These
contributions will be incident from behind the microphone (but not directly
behind) so they should have a relatively reduced magnitude.

The first contributions that strike the receiver from the front have path
lengths of 11m and 13m, which should correspond to strong impulses at 0.0324s
and 0.0382s.

These expectations are reflected in the results, shown in +@fig:cardioid.  The
impulse response is silent at the time of the direct contribution, 0.00882s.
The first significant level is recorded at 0.0147s, with the loudest
contributions seen at 0.0324s and 0.0382s. These results are consistent across
the spectrum, indicating that the microphone model is matched between both
simulation methods.

![The response measured at a cardioid microphone pointing away from the source.
The times marked, from left to right, are the direct contribution time, the
first reflection time, and the first and second *forward-incident* reflection
times.](images/cardioid){#fig:cardioid}

<div id="audio_table">

--------------------------------------------------------------------------------
test                            audio file
------------------------------- ------------------------------------------------
cardioid mic facing away        <audio controls><source src="demos/out_cardioid_away.mp3" type="audio/mpeg">This browser does not support html5 audio.</audio>
--------------------------------------------------------------------------------

</div>

### Binaural Modelling

Finally, the binaural model is tested. A concert hall is simulated, with a
source placed in the centre of the stage area. A receiver is placed 10m away
along both the x- and z-axes. The receiver is oriented so that it is facing
directly down the z axis, meaning that the source is 14.1m away, on the left of
the receiver.

The simulation produces output impulse responses for both the left and right
ears. All of Wayverb's simulation methods (image-source, ray-tracing and
waveguide) use HRTF data to produce stereo effects based on interaural level
difference. The image-source method additionally offsets the receiver position
to produce interaural time difference effects, so in the outputs, slight time
differences in the early reflections between channels are expected, and small
level differences should be seen throughout both files.

In particular, the direct contribution would normally arrive at
14.1/340=0.0416s. However, the left "ear" is actually slightly closer to the
source, and the right ear is slightly further away, which means that the first
contribution should be at 0.0414s in the left channel, and 0.0418s in the
right. The right ear is obstructed by the listener's head, and should therefore
have a reduced level relative to the left ear.

![Comparison of left- and right-ear responses, when the source is placed to the
left of the receiver. Note the amplitude and time differences between the early
reflections. The first contribution, in particular, is quieter in the right ear
because it is occluded by the listener's virtual
head.](images/binaural_signals)

The expected behaviour is observed in the outputs. The earliest contribution in
the left channel occurs at 0.0414s, and has a greater level than the right
channel's contribution at 0.148s. The left-channel early reflections have an
overall higher level than the early reflections in the right channel.  However,
as the impulse response progresses and becomes more diffuse, the energy levels
equalise between channels.

<div id="audio_table">

--------------------------------------------------------------------------------
test                            audio file
------------------------------- ------------------------------------------------
binaural receiver, source left  <audio controls><source src="demos/out_binaural.mp3" type="audio/mpeg">This browser does not support html5 audio.</audio>
--------------------------------------------------------------------------------

</div>

## Analysis

### Simulation Method

All models perform as expected with regards to changes in room size and shape,
material coefficients, source and receiver positions, and receiver microphone
type. Reverb features such as distinct late echoes can be generated, and
precise stereo effects, relying on both interaural time- and level-difference,
can be created. 

The main drawback, evident in several of the tests, is that the geometric and
wave-based models have distinct behaviours. In the room-size and material
tests, there are markedly different reverb times between the ray-tracer and
waveguide outputs; and in the obstruction test, the waveguide exhibits
diffraction behaviour which is not mirrored in the geometric output. These
differences lead to obvious discontinuities in the frequency response, which
persist despite calibrating the models to produce the same sound level at the
same distance, and implementing matching boundary models in all models.

Some differences are to be expected. The primary reason for implementing
multiple simulation methods is the relative accuracy of the different methods.
Geometric algorithms are known to be inaccurate at low frequencies, a
shortcoming that the waveguide was designed to overcome.

However, an accurate low-frequency response is useless if there is an obvious
disconnect between high- and low-frequency outputs. For music and sound design
applications, responses will sound *more* artificial if there is a rapid change
in the frequency response, even if the low-frequency response taken alone is an
accurate representation of the modelled space. Here, it is preferable that the
frequency response not contain obvious discontinuities, even if this
necessitates a reduction in overall accuracy.

Practical solutions to this problem are unclear. Ideally, the entire simulation
would be run with the waveguide method, but this is impractical for all but the
smallest simulations. Another option is to reduce the audible impact of the
crossover between the waveguide and geometric outputs, simply by increasing its
width.  This has the dual drawbacks of decreasing the low-frequency accuracy,
while also requiring a higher waveguide sample-rate, which is computationally
expensive.  Alternatively, the geometric algorithms may be altered, to account
for effects such as diffraction, with the goal of minimising the differences
between the wave and ray-based methods. This solution would maintain (or even
improve) the accuracy of the simulation, but would again increase its cost.
Finally, the goal of low-frequency accuracy could be abandoned, and the entire
spectrum modelled using geometric methods. However, this would prevent
important characteristics such as modal behaviour from being recorded.

### General Implementation

The implementation has several known issues, other than those shown in the
tests above.  These issues can broadly be separated into two categories:
firstly, problems with the simulation algorithm which were not highlighted in
the above tests; and secondly, usability issues with the program interface.

#### Algorithm

As noted in the [Digital Waveguide Mesh]({{ site.baseurl }}{% link waveguide.md
%}) section, the input signal used to excite the waveguide is not optimal. Its
frequency response extends up to the Nyquist frequency, which means that the
mesh has energy above the desired output frequency. As shown in the [Boundary
Modelling]({{ site.baseurl }}{% link boundary.md %}) section, the performance
of the boundary filters is often erratic above 0.15 of the mesh sampling rate,
sometimes increasing rather than reducing gain of incident signals. In
combination, the broadband input signal sometimes causes the reflection filters
to repeatedly amplify high-frequency content in the mesh. This is not audible
in the final results, as the high frequency content is filtered out. However,
it still leads to loss of precision, and sometimes numeric overflow. This might
be solved by high-pass filtering the input signal, and then deconvolving the
mesh output.  However, it is not clear how such a process would interact with
the microphone simulation.  For example, it would probably be necessary to
record and deconvolve the signals at all nodes surrounding the output node in
order to produce a correct intensity vector. This would require further
development and testing, for which there was insufficient time during the
Wayverb project.

A similar drawback is to do with the low-frequency response of the mesh. Most
input signals cause an increasing DC offset when the mesh uses a "soft" input
node. To solve this, Wayverb's mesh is excited using a "hard" node, which
introduces low-frequency oscillations and reflection artefacts. An important
area for future research is the development of an input signal which can be
used in conjunction with a soft source, which does not cause any DC offset.

An important feature which is not implemented in Wayverb is the modelling of
directional sources.  Currently, all modelled sources emit a single spherical
wave-front, which has equal energy in all directions. Real-world sources such
as musical instruments and the human voice are directional. The ability to
model directional sources would allow musicians and sound designers to create
much more realistic and immersive acoustic simulations.

As well as directional sources, it might be useful to make the implementation
of directional receivers more generic. Specifically, an ambisonic receiver
would be useful, so that simulation results could be exported for directional
processing in dedicated software. This could be achieved without modifying the
geometric microphone model, in which coincident capsules are well-justified and
lead to performance improvements (the simulation is run once for the entire
capsule group, instead of once per individual capsule). However, the approach
is not strictly justified in combination with the current waveguide microphone
model [@hacihabiboglu_simulation_2010]. Ambisonic output would therefore
require further research into waveguide microphone modelling, in order to find
a model which better supports coincident capsules.

The speed of the algorithm is lacking. The app was developed and tested on a
recent laptop with a 2.5GHz processor, 16GB of RAM, and an MD Radeon R9 M370X
graphics card with 2GB of dedicated VRAM. On this machine, several of the tests
above took five or even ten minutes to run, which is a little too slow to be
useful. When producing music, it is important to be able to audition and tweak
reverbs in order to produce the most appropriate effect. With long rendering
times, this auditioning process becomes protracted, which impacts the
productivity of the musician. In addition, if the program uses the majority of
the machine's resources while it is running, then this prevents the user from
running it in the background and continuing with other tasks in the meantime.

Alternatively, the speed may be acceptable for architectural applications,
where absolute accuracy is most important. In this scenario, the focus is upon
checking the acoustics of a previously-specified enclosure, rather than
experimenting to find the most pleasing output. This means that longer times
between renders can be justified if the results are highly accurate.
Additionally, users may have access to more powerful server or specialised
compute hardware which could lead to speed-ups.

The simulation methods have been implemented to minimise average-case
computational complexity wherever possible, but both the ray-tracing and
waveguide processes are limited to a complexity of $O(n)$ where n refers to the
number of rays or nodes required in the simulation.  Any further performance
improvements may only be gained by improving the per-ray or per-node processing
speed. This is certainly possible, but would yield relatively small
improvements to the simulation speed. It may be equally valid to simply wait
for hardware with increased parallelism support: a machine with twice as many
graphics cores will run the program twice as fast. Such machines are likely to
be commonplace in two or three years. Therefore, a better use of time would be
to spend those two years focusing on the algorithm's functional problems
rather than optimisation.

#### User Interface

The user interface is less fully-featured than may be expected of a
professional simulation program.  The reason for this is simple: the entire
application was developed by a single developer, over sixteen months. To ensure
that the application would reach a usable state, its scope had to be limited.
In its first release, the application is only capable of loading, saving,
configuring, and running simulations.

Originally, the app was designed to include built-in convolution support, so
that generated impulse responses could be previewed without leaving the
application. This feature would greatly improve the usability of the program.
However, it would not contribute to the main goal of the program, which is the
accurate and fast acoustic simulation of virtual environments.  Convolution
reverb tools already exist, and many users will have their own favourite
programs and plug-ins for this purpose. The time that would have been spent
replicating this functionality was better spent working on the unique and novel
features of the program.

Similarly, the ability to edit the virtual spaces themselves from within the
app was not implemented. Writing an intuitive editor for 3D objects would be a
large undertaking, even for a team of developers.  Instead, the ability to load
a variety of 3D file formats was included, and users are advised to use
dedicated software such as Blender or Sketchup to create their simulated
spaces.

Some further usability features which are missing, which would ideally be
included in a future release, include:

- **Undo and redo**: If the user accidentally moves a source or receiver, or
  makes some other unwanted change, they must revert the change manually.  There
  is no way of automatically reverting to the previous program state.
- **Load and save of capsule and material presets**: If the model contains
  several surfaces with different materials, and the user wants to apply a
  specific set of coefficients to all surfaces, each material must be configured
  by hand. There is no way to copy coefficients between surfaces, or to save and
  load materials from disk. Similarly, there is no way to save and load complex
  receiver set-ups from disk.
- **Improved visualisation**: Currently, ray energies are not communicated via
  the visualisation. There is also no way of adjusting the gain of the waveguide
  visualisation, which means that often the waveguide energy dies away quickly,
  becoming invisible, and giving the false impression that the simulation is not
  progressing.
- **Command-line interface**: For scripting or batch-processing of simulations,
  it would be useful to be able to run simulations from the command-line.
  Currently, this is only made possible by writing custom wrapper programs for
  the Wayverb library. It would be more useful to integrate command-line options
  directly into the Wayverb program.

Finally, it was not possible to test the program extensively for crashes and
bugs. The program was tested on a 15-inch MacBook Pro running OS 10.11, and a
little on earlier models of 15- and 13-inch Macbook Pros running OS 10.10. On
10.11, and on the 13-inch laptop running 10.10, no crashes were evident,
although on the 15-inch 10.10 machine there were a few crashes within OpenCL
framework code. These crashes were reported by a user, from their personal
machine. There was not sufficient time to fix these bugs during the project.

Extended access to this machine was not possible, and debugging OpenCL code
without access to the problematic hardware is difficult. Depending on the
drivers supplied by the GPU vendor, the kernel may be compiled in subtly
different ways. For most (non-OpenCL) software, there will be a single binary
image for a given processor architecture, and if the program crashes then a
stack trace can be used to find the location of the bug.  However, for OpenCL
code, the executed binary is generated at runtime, and will vary depending on
the specification and driver of the GPU.  Also, crashes inside the OpenCL
kernel do not emit a stack trace.  Therefore, it is almost impossible to debug
OpenCL code without access to the specific machine configuration which causes
the issue.

A future release could fix these problems, but only with access to any
problematic hardware and software configurations. As the program is open-source
it would also be possible for third-parties experiencing bugs to contribute
fixes.
