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
highly directional, containing direct sound from the direction of the source,
and reflected sounds from the room boundaries. To produce a convincing virtual
environment, these directional cues must be recorded and reproduced.
Omnidirectional microphones cannot record directional responses, and so a
different direction-sensitive receiver model must be developed and used
instead.

## Background

Humans are able to detect the originating direction of sounds using two main
techniques: Interaural Time Difference (ITD) defines the time delay between
ears, when the sound source is located nearer to one ear than the other; and
Interaural Level Difference (ILD) denotes the frequency-dependent difference in
level caused by attenuation by the outer ear, head, and torso.  The success of
virtual acoustics depends on the ability to produce signals with appropriate
ITD and ILD, in such a way that the sound is perceived as coming from a certain
direction.

When recording impulse responses of physical spaces, several techniques might
be used to retain ITD and ILD information.

One option is to record the impulse response using a matched pair of
microphones.  A spaced pair of omnidirectional microphones (known as an AB
pair) will capture interchannel time difference, but will not capture
interchannel level distance if the source is positioned far away from the
microphones. Alternatively, a coincident pair of cardioid capsules (an XY pair)
or bidirectional capsules (a Blumlein) pair might be used. These configurations
capture level difference, but are incapable of recording time difference,
because sound will always arrive at both capsules at the same time.

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
receiver model should encompass all of the techniques described above. Given
that spaced microphone techniques can be modelled simply by including multiple
receivers in the simulation, each virtual capsule should have configurable
direction-dependent gain and frequency response, allowing it to behave like a
standard microphone, or like the ear of a dummy head.

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
pre-computed table.

It is also easy to adapt this technique, in the case that the frequency
response of the receiver depends on incident direction, rather than just the
overall gain.  In Wayverb, image-source contributions are calculated in eight
frequency bands, which are filtered and mixed down at the end of the
simulation.  A direction-dependent frequency response can be modelled simply by
using a different attenuation function or look-up table for each frequency
band.  Using this method, interchannel level difference can be modelled with
accuracy matching the rest of the image-source simulation.

<!-- TODO mention interchannel time difference method for image source? -->

## Ray Tracer Implementation

In the ray tracer, a very similar approach can be taken.  The incident
direction of each ray is known, so this direction can be used with a function
or lookup-table to adjust the ray's multiband intensity before it is added to
the energy histogram (see [Ray Tracer]({{ site.baseurl }}{% link ray_tracer.md
%})).

In terms of implementation, this approach is very expensive.  In Wayverb, the
initial simulation and directional processing stages are kept separate,
allowing a single simulation to be processed by several different virtual
microphone capsules. While flexible, it is also infeasible to store the
direction of each incident ray, especially if there are hundreds of thousands
of rays in the simulation. The ray tracer is used to estimate the late
reverberation of the space, in which individual sonic events are "blurred"
together, and in which there are few directional cues
[@schroder_physically_2011, p. 21], so this level of per-ray directional
accuracy is unnecessary.

Instead, the surface of the spherical receiver is divided into discrete
*directivity groups* [@schroder_physically_2011, p. 72]. A separate energy
histogram is maintained for each directivity group, where the direction of ray
incidence determines the histogram used to store that ray's energy.

<!-- TODO raytracer process diagram -->

<!-- TODO tests? -->

## DWM Implementation

TODO 

* what were the implementation options and decisions

* why did I choose the final implementation

* how does it work

* tests - *does* it work?
