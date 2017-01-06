---
layout: page
title: Context
navigation_weight: 1
---

---
reference-section-title: References
...

# Context {.major}

## Overview

Room acoustics algorithms fall into two main categories: *geometric*, and
*wave-based* [@southern_spatial_2011].  Wave-based methods aim to solve the
wave equation numerically, simulating the actual behaviour of sound waves
within an enclosure.  Geometric methods instead make some simplifying
assumptions about the behaviour of sound waves, which result in faster but less
accurate simulations.  These assumptions generally ignore all wave properties
of sound, choosing to model sound as independent *rays*, *particles*, or
*phonons*.

The modelling of waves as particles has found great success in the field of
computer graphics, where *ray-tracing* is used to simulate the reflections of
light in a scene.  The technique works well for simulating light because of the
relatively high frequencies of the modelled waves.  The wavelengths of these
waves - the wavelengths of the visible spectrum - will generally be many times
smaller than any surface in the scene being rendered, so wave phenomena have
little or no visible effect.

The assumption that rays and waves are interchangeable falls down somewhat when
modelling sound.  The wavelengths of sound in air range from 17m to 0.017m for
the frequency range 20Hz to 20KHz, so while the simulation may be accurate at
high frequencies, at low frequencies the wavelength is of the same order as the
wall surfaces in the scene.  Failure to take wave effects such as interference
and diffraction into account at these frequencies therefore results in
noticeable approximation error [@savioja_overview_2015].

In many cases, some inaccuracy is an acceptable (or even necessary) trade-off.
Wave-modelling is so computationally expensive that using it to simulate a
large scene over a broad spectrum could take weeks on consumer hardware.  This
leaves geometric methods as the only viable alternative.  Though wave-modelling
been studied for some time [@smith_physical_1992], and even applied to small
simulations of strings and membranes in consumer devices such as keyboards, it
is only recently, as computers have become more powerful, that these techniques
have been seriously considered for room acoustics simulation.

Given that wave-based methods are accurate, but become more expensive at higher
frequencies, and that geometric methods are inexpensive, but become less
accurate at lower frequencies, it is natural to combine the two models in a way
that takes advantage of the desirable characteristics of each.  That is, by
using wave-modelling for low-frequency content, and geometric methods for
high-frequency content, simulations may be produced which are accurate across
the entire spectrum, without incurring massive computational costs.

## Characteristics of Simulation Methods

A short review of simulation methods will be given here.  For a detailed survey
of methods used in room acoustics, see [@svensson_computational_2002].

The following figure \text{(\ref{fig:simulation_techniques})} shows the
relationships between the most common simulation methods.  The advantages and
disadvantages of each method will be discussed throughout the remainder of this
section.

![An overview of different acoustic simulation methods, grouped by
category.\label{fig:simulation_techniques}](images/simulation_techniques)

### Geometric Methods

Geometric methods can be grouped into two categories: *stochastic* and
*deterministic*. 

Stochastic methods are generally based on statistical approximation via some
kind of Monte Carlo algorithm.  Such algorithms are approximate by nature.
They aim to randomly and repeatedly sample the problem space , combining the
results from multiple trials so that they converge upon the correct answer.
The balance of quality and speed can be adjusted in a straightforwward manner,
simply by adjusting the number of samples taken.

In room acoustics, stochastic algorithms may be based directly on reflection
paths, using *ray tracing* or *beam tracing*, in which rays or beams are
considered to transport acoustic energy around the scene.  Alternatively, they
may use a surface-based technique, such as *acoustic radiance transfer* (ART),
in which surfaces are used as intermediate stores of acoustic energy.

Surface-based methods, especially, are suited to real-time simulations (i.e.
interactive, where the listener position can change), as the calculation occurs
in several passes, only the last of which involves the receiver object.  This
means that early passes can be computed and cached, and only the final pass
must be recomputed if the receiver position changes.

The main deterministic method is the *image source* method, which is designed
to calculate the exact reflection paths between a source and a receiver.  For
shoebox-shaped rooms, and perfectly rigid surfaces, it is able to produce an
exact solution to the wave equation.  However, by its nature, it can only model
specular (perfect) reflections, ignoring diffuse and diffracted components.
For this reason, it is inexact for arbitrary enclosures, and unsuitable for
calculating reverb tails, which are predominantly diffuse.  The technique also
becomes prohibitively expensive beyond low orders of reflection.  The naive
implementation reflects the sound source against all surfaces in the scene,
resulting in a set of *image sources*.  Then, each of these image sources is
itself reflected against all surfaces.  For high orders of reflection, the
required number of calculations quickly becomes impractical.  For these
reasons, the image source method is only suitable for early reflections, and is
generally combined with a stochastic method to find the late part of an impulse
response.

For a detailed reference on geometric acoustic methods, see
[@savioja_overview_2015].

### Wave-based Methods

The main advantage of wave-based methods is that they inherently account for
wave effects like diffraction and interference [@shelley_diffuse_2007], while
geometric methods do not.  This means that these wave-based methods are capable
of accurately simulating the low-frequency component of a room
impulse-response, where constructive and destructive wave interference form
*room modes*.  Room modes have the effect of amplifying and attenuating
specific frequencies in the room impulse response, and produce much of the
subjective sonic "colour" or "character" of a room.  Reproducing these room
modes is therefore vital for evaluating the acoustics of rooms such as concert
halls and recording studios, or when producing musically pleasing reverbs.

Wave-based methods may be derived from the *Finite Element Method* (FEM),
*Boundary Element Method* (BEM) or *Finite-Difference Time-Domain* (FDTD)
method. The FEM and BEM may be known together as *element methods*.

The FEM is an iterative numerical method for finding natural resonances of a
bounded enclosure.  It models the air pressure inside the enclosure using a
grid of interconnected nodes, each of which represents a mechanical system with
a single degree of freedom.  The interconnectedness of the nodes leads to a set
of simultaneous equations, which can be solved for displacement at each node,
and then the solved equations can be used to calculate pressure values at
certain elements.  The BEM is similar, but models nodes on the surface of the
enclosure, instead of within it.  This in turn allows it to model unbounded
spaces. [@murphy_digital_2000]

The FDTD method works by dividing the space to be modelled into a regular grid,
and computing changes in some quantity at each grid point over time.  The
formula used to update each grid point, along with the topology of the grid,
may be varied depending on the accuracy, efficiency, and complexity required by
the application.  FDTD methods are generally applied to problems in
electromagnetics, but a subclass of the FDTD method known as the *Digital
Waveguide Mesh* (DWM) is often used for solving acoustics problems.

The FDTD process shares some characteristics with the element methods.  They
all become rapidly more computationally expensive as the maximum output
frequency increases [@valimaki_fifty_2012].  They also share the problem of
discretisation or quantisation, in which details of the modelled room can only
be resolved to the same accuracy as the spatial sampling period.  If a large
inter-element spacing is used, details of the room shape will be lost, whereas
a small spacing will greatly increase the computational load.

The major advantage of FDTD over element methods is that it is run directly in
the time domain, rather than producing frequency-domain results, which in turn
affords a much simpler implementation.

The main disadvantage of the FDTD method is that it is susceptible to
*numerical dispersion*, in which wave components travel at different speeds
depending on their frequency and direction, especially at high frequencies.
Several techniques exist to reduce this error, such as oversampling the mesh
[@campos_computational_2005], using different mesh topologies
[@savioja_reduction_1999; @van_duyne_tetrahedral_1995], and post-processing the
simulation output [@savioja_interpolated_2001].  Oversampling further increases
the computational load of the simulation, while using different topologies and
post-processing both introduce additional complexity.

Despite its drawbacks, the FDTD method is generally preferred for room
acoustics simulation [@valimaki_fifty_2012], probably due to its
straightforward implementation, intuitive behaviour, and its ability to
directly produce time-domain impulse responses.

TODO note that FDTD is embarrassingly parallel

## Existing Software

Searching online and in the literature uncovers a handful of programs for
acoustic simulation.  The following table \text{(\ref{tab:software})} shows a
selection which is not exhaustive, but which is felt to be representative.

Table: Some of the most prominent tools for acoustic
simulation.\label{tab:software}

Name                                  | Type                        | Availability
--------------------------------------|-----------------------------|------------------
Odeon [@_odeon_2016]                  | Geometric                   | Commercial       
CATT-Acoustic [@_catt-acoustic_2016]  | Geometric 					| Commercial 
Olive Tree Lab [@_otl_2016]           | Geometric 					| Commercial 
EASE [@_ease_2016]                    | Geometric 					| Commercial 
Auratorium [@_audioborn_2016]         | Geometric 					| Commercial 
RAVEN [@schroder_raven:_2011]         | Geometric 					| None 
RoomWeaver [@beeson_roomweaver:_2004] | Waveguide                   | None 
EAR [@_ear_2016]                      | Geometric                   | Free
PachydermAcoustic [@_pachyderm_2016]  | Geometric                   | Free
Parallel FDTD [@_parallelfdtd_2016]   | Waveguide                   | Free
i-Simpa [@_i-simpa_2016]              | Geometric, extensible       | Free

All commercial acoustics programs found use geometric techniques, probably
because they are fast to run, and can often be implemented to run
interactively, in real-time.  However, low-frequency performance is a known
issue with these programs.  For example, the FAQ page for the Odeon software
[@_odeon_2016-1] notes that:

> For Odeon simulations as with real measurements, the source and receiver
should be at least 1/4th wave length from the walls. But at the very lowest
resonance of the room the level can change a lot from position to position
without Odeon being able to predict it. For investigation of low frequency
behavior (resonances), indeed Odeon is not the tool. 

Clearly there is a need for wave-modelling acoustics software, which can
accurately predict low frequency behaviour.  However, such software seems to be
somewhat rarer than geometric acoustics software.  Of the two wave-modelling
programs listed, only one is generally available, which must additionally be
run from Python or Matlab scripts.  This is a good approach for research
software, but would probably not be straightforward for users with limited
programming experience.

At time of writing, December 2016, it appears that no generally-available
(commercially or otherwise) piece of software has taken the approach of
combining wave-modelling and geometric methods, although this technique is
well-known in the literature [@southern_hybrid_2013; @aretz_combined_2009;
@murphy_hybrid_2008; @southern_room_2013; @vorlander_simulation_2009;
@southern_spatial_2011].

## Research Aims

With the preceding context in mind, it appears that there are obvious benefits
to a program which combines geometric and wave-based methods to produce
simulations which are accurate across the audible spectrum. Rather than
focussing on performance, or interactive simulation (which is already
implemented in the commercial software above), such a program should strive
towards accuracy first, and performance second.  To be useful to end-users, the
program should have a graphical interface, though a scripting or library
interface might be provided for research purposes.  Finally, the program would
ideally be free and open-source, to maximise adoption and to aid future
research and collaboration.  The goal of the Wayverb project was to produce a
program which satisfied these requirements.

At time of writing, Wayverb is the only public graphical acoustics tool
incorporating geometric and wave-based methods.  Although hybrid acoustics
methods are well documented [@southern_hybrid_2013; @aretz_combined_2009;
@murphy_hybrid_2008], they have only been used in specific research settings,
for producing experimental results. It may be assumed that these tools have
been built to model specific test-cases, rather than general simulation tasks,
but this is uncertain as no tools incorporating these techniques have been made
public. Wayverb is able to model arbitrary enclosures, and has the additional
benefits of a graphical interface (making it more accessible to a wider
audience) and an open-source code-base (allowing it to be modified by other
researchers).

The notable components of the Wayverb project are as follows, each of which has
a dedicated chapter with detailed explanation:

* Image-source model, accelerated with parallel ray-casting, for early
  reflections. Uses a novel method for speeding up audibility tests by re-using 
  reflection paths from the ray tracer.
* Parallel stochastic ray-tracer, for late reflections.
* Parallel digital waveguide mesh, for low frequency modelling. Uses a novel
  set-up procedure to create meshes with correctly-placed boundary nodes in 
  arbitrary scenes.
* Calibration, automatically matching the output levels of the different 
  models.
* A microphone model, capable of simulating capsules with direction-dependent 
  frequency responses, within all three simulation-types.
* A boundary model with matched performance in all three simulation-types.

The project acts as a survey of room acoustics techniques.  Rather than
designing completely new simulation methods, existing techniques were
investigated, compared, and evaluated in terms of their accuracy and
performance. Then, optimum techniques could be chosen and further developed for
use in the final program. An especially important consideration is the
matching of parameters between models. For example, all models should produce
the same sound energy at a given distance, and should exhibit the same reverb
time for a given scene. Therefore, the acoustics techniques were chosen so that
they produce consistent results.

Sometimes the models required development beyond the methods presented in the
literature in order to become useful. An example of this is the waveguide
set-up process. Most experimental set-ups in the literature only model
cuboid-shaped enclosures, and no guidance is given for setting up simulations
in arbitrarily-shaped enclosures. Of course, it must be possible to model real,
complex room shapes, and so an original set-up procedure had to be developed.
The same goes for memory layout and implementation details: in the literature,
techniques for efficient implementation are rarely discussed. As a result, new
techniques had to be invented, rather than reimplementing known methods.  Where
extensions to existing techniques have been developed for use in Wayverb, this
is mentioned in the text.

Much of the literature on acoustic simulation focuses predominantly on
accuracy.  Performance appraisals are rarely given, presumably because they are
somewhat subjective, and "reasonable" efficiency will vary between
applications.  Ideally, the simulation methods in Wayverb should be selected
and implemented to allow tunable performance, so that results with acceptable
accuracy can be generated within a few minutes, but it is possible to run
longer simulations if higher-accuracy results are needed.  This is similar to
approaches taken in computer graphics, where "overview" renders may take
seconds to generate, but physically-modelled simulations for film often take
hours to render, even on purpose-built compute clusters.

## Strategy

### Chosen Simulation Techniques

As the image-source method is well-suited to finding early reflections, and
stochastic methods are reasonably accurate at computing the more diffuse late
reflections, it made sense to combine these two methods for high-frequency
simulation.  Specifically, a simple ray tracing method was chosen over a
phonon- or surface-based method for the late-reflection simulation, for two
reasons.  Firstly, ray tracing is broadly discussed in the literature
[@krokstad_calculating_1968; @kuttruff_room_2009;
@vorlander_auralization:_2007; @schroder_physically_2011;
@alpkocak_computing_2010], so would not require a great deal of experimentation
to implement.  Secondly, ray tracing has the property of being an
*embarrassingly parallel* algorithm, because each individual ray can be
simulated entirely independently, without requiring communication or
synchronisation.  By running the algorithm on graphics hardware, which is
designed to run great numbers of calculations in parallel, all rays could be
simulated in one go, yielding much greater performance than processing each ray
sequentially.

A logistical reason for choosing the image-source and ray tracing solution for
high-frequency modelling was that the author had previously implemented such a
system for an undergraduate project.  It was hoped that much of the code from
that project could be re-used, but it transpired that much of the code was
unsuitable or incorrect (!), so that the majority was completely re-written.
The author was, however, able to re-use much of the knowledge and experience
gained from the previous project, which would not have been possible if a
completely new stochastic method had been introduced.

For low-frequency simulation, a FDTD-based DWM model was chosen.  There is a
great deal of writing on this method [@van_duyne_3d_1996;
@savioja_interpolated_2014; @kowalczyk_room_2011; @campos_computational_2005;
@murphy_digital_2000], it is relatively simple to implement, and shares with
ray tracing the characteristic of being embarrassingly parallel.  Each element
in the waveguide mesh can be updated individually and simultaneously, which it
was hoped would yield performance benefits.

An in-depth description of the algorithms implemented is given in the
[Image-Source]({{ site.baseurl }}{% link image_source.md %}), [Ray Tracer]({{
site.baseurl }}{% link ray_tracer.md %}), and [Waveguide]({{ site.baseurl }}{%
link waveguide.md %}) sections.  The following figure
\text{(\ref{fig:regions})} shows how the outputs from the different methods
work together to produce a broadband impulse response.

![The structure of a simulated impulse
response.\label{fig:regions}](images/regions)

Deciding on the simulation techniques led to three questions:

* To produce a final output, the three simulations must be automatically mixed
  in some way.  How can this be done? 
* Binaural simulation requires some method for direction- and
  frequency-dependent attenuation at the receiver.  How can receivers with polar
  patterns other than omnidirectional be modelled consistently in all three
  simulation methods?  
* The reverb time and character depends heavily on the nature of the reflective
  surfaces in the scene.  How can frequency-dependent reflective boundaries be
  modelled consistently in all methods?

These questions will be discussed in the [Hybrid]({{ site.baseurl }}{% link
hybrid.md %}), [Microphone Modelling]({{ site.baseurl }}{% link microphone.md
%}), and [Boundary Modelling]({{ site.baseurl }}{% link boundary.md %})
sections respectively.

### Chosen Technology

The programming language chosen was C++.  For acceptable performance in
numerical computing, a low-level language is required, and for rapid
prototyping, high-level abstractions are necessary.  C++ delivers on both of
these requirements, for the most part, although its fundamentally unsafe memory
model does introduce a class of bugs which do not really exist in languages
with garbage collection, borrow checking, or some other safety mechanism.

OpenCL was chosen for implementing the most parallel parts of the simulation.
The OpenCL framework allows a single source file to be written, in a C-like
language, which can target either standard *central processing units* (CPUs),
or highly parallel *graphics processing units* (GPUs).  The main alternative to
OpenCL is CUDA, which additionally can compile C++ code, but which can only
target Nvidia hardware.  OpenCL was chosen as it would allow the final program
to be run on a wider variety of systems, with fewer limitations on their
graphics hardware.

The only deployment target was macOS.  This was mainly to ease development, as
maintaining software across multiple platforms is often time-consuming.  macOS
also tends to have support for newer C++ language features than Windows.
Visual Studio 2015 for Windows still does not support all of the C++11 language
features [@_visual_2016], while the Clang compiler used by macOS has supported
newer C++14 features since version 3.4 [@_clang_2016], released in May 2014
[@_download_2016].  Targeting a single platform avoids the need to use only the
lowest common denominator of language features.  As far as possible, the
languages and libraries have been selected to be portable if the decision to
support other platforms is made in the future.  Once Windows fully supports
C++14, it should be possible to port the program with a minimum of effort.

The following additional libraries were used to speed development.  They are
all open-source and freely available.

GLM
:   Provides vector and matrix primitives and operations, primarily designed
for use in 3D graphics software, but useful for any program that will deal with
3D space.

Assimp 
:   Used for loading and saving 3D model files in a wide array of formats, with
a consistent interface for querying loaded files. 

FFTW3 
:   Provides Fast Fourier Transform routines.  Used mainly for filtering and
convolution.

Libsndfile 
:   Used for loading and saving audio files, specifically for saving simulation
results.

Libsamplerate 
:   Provides high-quality sample-rate-conversion routines.  Waveguide
simulations are often run at a relatively low sample-rate, which must then be
adjusted.

Gtest 
:   A unit-testing framework, used to validate small individual parts of the
program, and ensure that changes to one module do not cause breakage elsewhere.

Cereal 
:   Serializes data to and from files.  Used for saving program configuration
options.

ITPP 
:   A scientific computing library.  Used for its implementation of the
Yule-Walker method for estimating filter coefficients for a given magnitude
response.

JUCE 
:   Provides a framework for building graphical applications in C++.  Used for
the final application.

The project uses CMake to configure its build, and to automatically download
project dependencies.  Python and Octave were used for running and automating
tests and generating graphs.

This documentation is written in Markdown, and compiled to html and to pdf
using Pandoc.  The project website is generated with Jekyll.
