---
layout: page
title: Waveguide
navigation_weight: 4
---

--- 
reference-section-title: References 
...

# Digital Waveguide Mesh {.major}

## Background

The *digital waveguide mesh* (DWM) is one of several wave-based simulation
techniques.  Each technique in this family is derived directly from the wave
equation, allowing them to inherently support wave phenomena such as
diffraction and interference.  Wave effects such as these are particularly
important to the low-frequency response of a room.  This means that at low
frequencies wave-based methods are far more accurate than geometric methods,
which are not able to model wave effects [@southern_spatial_2011].

The drawback of wave-based methods is that their computational complexity
increases rapidly with the maximum output frequency, and with the size of the
modelled space.  On current consumer hardware, it is not feasible to compute a
full-spectrum simulation using wave-based techniques.  To optimise computation
time, while retaining reasonable accuracy across the spectrum, wave-based
methods can be combined with geometric methods.  Wave-based methods are used to
calculate accurate low-frequency content, while geometric methods can estimate
the higher frequency content, which is less dependent upon wave effects.

There are, largely speaking, two main types of wave-based simulation used for
room acoustics: element methods, and finite difference methods.  The waveguide
mesh is the latter, a simplified sub-class of the *finite-difference
time-domain* (FDTD) technique.  Although the DWM and FDTD have converged over
time, and are equivalent [@smith_iii_equivalence_2004;
@karjalainen_digital_2004], their histories are quite different.  The DWM was
designed to be efficient for small-scale acoustic simulations in which the only
quantity of interest is pressure [@smith_physical_1992], while FDTD is a more
general technique designed for electromagnetic simulation, in which the
electric and magnetic fields are both of interest [@botts_integrating_2013].

### Method

The derivation of the waveguide mesh begins with the one-dimensional *digital
waveguide*.  A one-dimensional waveguide can exactly describe the behaviour of
a band-limited wave in one dimension.  Such a model is well-suited for
predicting the behaviour of certain musical instruments which use columns of
air or vibrating strings to produce sound.  The model itself is based on
d'Alembert's solution of the wave equation in one dimension
[@shelley_diffuse_2007, p. 86].  The displacement of a string $y$ at time $t$
and position $x$ can be written as

(@) $$y(t,x)=y_r\left(t-\frac{x}{c}\right) + y_l\left(t+\frac{x}{c}\right)$$

where $y_r\left(t-\frac{x}{c}\right)$ and $y_l\left(t+\frac{x}{c}\right)$ are
the right- and left-going travelling waves respectively, with speed $c$.  The
discrete form of this equation is given in terms of constant time and space
divisions $T$ and $X$:

(@) $$y(nT,mX) \buildrel \Delta \over = y^+(n-m) + y^-(n+m)$$

where superscript $+$ and $-$ denote propagation to the right and left
respectively, and $n$ and $m$ are integral [@smith_iii_equivalence_2004].

An implementation of these equations will take the form of two parallel delay
lines, which propagate wave components in opposite directions.  This is shown
in the following diagram \text{(\ref{fig:one_d_waveguide})}.  The "output" of
the simulation, that is, the physical displacement of the modelled string, is
found by adding the wave components in both delay lines at a single point.

TODO nice diagram

Waveguides in higher dimensions can be created in a straightforward manner, by
connecting digital waveguide elements at a *scattering junction*.  Wave
components entering the junction are distributed amongst the waveguide
elements, preserving energy and power according to Kirchoff's power
conservation laws [@shelley_diffuse_2007, p. 87].  The sound pressure $p_J$ at
a lossless scattering junction $J$ with $N$ connected elements or "ports" is
the summed incoming components of all connected elements:

(@)
$$p_J=\frac{2\sum_{i=1}^{N}\frac{p_i^+}{Z_i}}{\sum_{i=1}^{N}\frac{1}{Z_i}}$$

where $p_i$ is the pressure in waveguide element $i$ and $Z_i$ is its
associated impedance.  This simplifies, if all impedances are equal, which is
true for homogeneous media:

(@) $$p_J=\frac{2}{N}\sum_{i=1}^{N}p_i^+$$

A *digital waveguide mesh* is any configuration of regularly-arranged $N$-port
scattering junctions which are separated by unit delay lines.  In some
literature, this specific type of mesh is known as a *W-DWM* because it
operates in terms of "W" (wave) variables.

The input into a scattering junction is equal to the output of a neighbour
junction at the previous time step.  This fact allows the waveguide mesh to
alternatively be formulated directly in terms of the pressure at each junction
(assuming all junction impedances are equal) [@beeson_roomweaver:_2004]:

(@) $$p_J(n)=\frac{2}{N}\sum_{i=1}^{N}p_i(n-1)-p_J(n-2)$$

That is, the next pressure at a given node depends on the previous pressure at
that node, and the current pressure at surrounding nodes.  This alternative
formulation operates on Kirchhoff variables, and is therefore known as a
*K-DWM*.  In the 1D case, the K-DWM and W-DWM are computationally identical
[@smith_iii_equivalence_2004].  In higher dimensions, they are equivalent only
under certain circumstances [@murphy_acoustic_2007, p. 5].

The K-DWM is advantageous compared to the W-DWM for reasons of efficiency.  It
requires less memory, and fewer calculations per node per step than the W-DWM:
experiments in [@beeson_roomweaver:_2004] show that the K-DWM is 200% faster
and uses 50% of the memory of the equivalent W-DWM.  This is mainly to do with
the number of values stored per node.  Each node in a K-DWM must store a
"current" and a "previous" pressure value, whereas in a W-DWM each *connection*
must store a value [@karjalainen_digital_2004].  For mesh layouts in which each
node has many neighbours (see [Mesh Topology] below), the W-DWM can require
many times more memory.  The K-DWM also requires one fewer addition per node
per step [@shelley_diffuse_2007, p. 91], so will be slightly faster, all else
being equal.

In the general case of an $N$-dimensional waveguide mesh, the spatial and
temporal sampling periods are related by the Courant number $\lambda$.  The
Courant criterion specifies the conditions required for numerical stability of
the simulation:

(@) $$\lambda=\frac{cT}{X} \leq \frac{1}{\sqrt{N}}$$

The highest sampling rate and lowest error is achieved by setting the Courant
to its maximum value [@sheaffer_fdtd/k-dwm_2010].  This is normally desirable,
and so the inequality above can be simplified:

(@) $$T=\frac{X}{c\sqrt{N}}$$

Note that a higher output sampling rate requires a smaller inter-nodal spacing
and therefore more modelled points per-unit-volume, which in turn requires more
memory and more calculations per time step.  Unfortunately, the valid bandwidth
of the output is generally smaller than the maximum available bandwidth, i.e.
up to the Nyquist frequency [@kowalczyk_room_2011].  The waveguide sampling
rate must generally be at least four times the maximum required output
frequency for higher-dimensional simulations (more detailed bandwidths for some
common topologies are given in [@kowalczyk_room_2011]).

### Strengths and Weaknesses of the DWM

The main advantage of the DWM is its relative simplicity.  The air in the
simulation is evenly divided into nodes.  Each node has an associated pressure,
and also stores its previous pressure.  The next pressure at a node depends on
its previous pressure, and the current pressures at its neighbours.  The
simulation progresses by repeatedly updating all nodes, and storing the change
in pressure over time at some output nodes.

This simplicity presents an optimisation opportunity.  Each node can be updated
completely independently, as long as all updates in one time-step are completed
before the next time-step begins.  This means that the updates can happen in
parallel.  In fact, with enough memory and processing cores, the entire mesh
could be updated in the time that it takes to update a single node.  The update
method is also very simple, only requiring some additions and a multiplication
per node per step.  This kind of simple, parallel code lends itself to
implementations on graphics hardware, which is designed for running similar
calculations simultaneously over large inputs.  *Graphics processing units*
(GPUs) have best throughput when all threads execute the same instruction, and
when memory is accessed at consecutive addresses from running threads.  The
K-DWM update equation has no branching, and has consistent memory access
patterns, which should allow for a very efficient implementation for GPUs.

The greatest limitation of the DWM is *dispersion error*.  Unlike waves in
homogeneous physical media, the velocity of wave propagation in the DWM depends
on the direction of propagation, and also on the frequency of the wave
component.  This leads to errors in the frequency response of recorded signals,
especially toward the upper limit of the output bandwidth.  The exact pattern
of dispersion error is dependent upon the topology of the mesh, and can be
examined using *Von Neumann* analysis [@van_duyne_3d_1996].  One solution to
the dispersion problem is to increase the sampling rate of the mesh, moving the
high-error area out of the region of interest.  Of course, this can quickly
become very expensive, as the number of nodes, and therefore memory usage and
computation time is proportional to the inverse cube of the sampling period.
Another option is to use a mesh topology designed to reduce both direction- and
frequency-dependent error, such as those presented in [@kowalczyk_room_2011].
One interesting variation on this option is to reduce only direction-dependent
error, and then to compensate for frequency-dependent error with a
post-processing step, which is the approach taken in
[@savioja_interpolated_2001].  However, these mesh topologies with higher
accuracy and isotropy require relatively high numbers of calculations per node.
The interpolated schemes in [@kowalczyk_room_2011] require 27 additions and 4
multiplications per node, whereas a tetrahedral mesh would require 5 additions
and 1 multiplication.  It is clear that high-accuracy results will be very
costly to compute, whichever method is used.

<!-- TODO how problematic are stepped boundaries? -->

## Design Choices

### Mesh Type

#### Mesh Topology

There is no single optimal implementation of the digital waveguide mesh.
Perhaps the most important decision is the mesh topology or *stencil* that will
be used, and by extension the mesh update equation.  Here, the mesh topology
refers to the pattern which is used to distribute nodes throughout the modelled
space.  The choice of topology will affect the accuracy, memory usage,
calculation speed, and implementation complexity of the final design.  It must
therefore be chosen with care, in order to satisfy the constraints of the
particular application.

The simplest topology is rectilinear, in which nodes are laid out on the
vertices of a cubic grid, and each node has 6 direct neighbours.  During the
mesh update, the pressure at each neighbour node must be checked in order to
calculate the next pressure at the current node.  This is straightforward to
implement, as the nodes can be stored in memory in a three-dimensional array,
in which the array extents define the mesh dimensions, and the array indices
refer to the positions of individual nodes.  Other options for the topology
include tetrahedral, octahedral, and dodecahedral, in which nodes have 4, 8,
and 12 neighbours respectively.

TODO topology diagrams

The accuracy may be increased by overlaying or "superposing" cubic, octahedral,
and dodecahedral schemes together, as all nodes are oriented uniformly, and
have cubic tessellation.  Such schemes are known as *interpolated*, and in
these schemes each node has 26 neighbours.  The cubic, octahedral,
dodecahedral, and interpolated schemes may additionally all be represented by a
single "unified" update equation, described in [@kowalczyk_room_2011].  In this
respect the tetrahedral scheme is unique, requiring a dedicated update method.
This is because the nodes in a tetrahedral mesh may be oriented in either of
two directions, effectively requiring two update equations instead of one.

If the primary concern is speed rather than accuracy, a scheme with fewer
neighbour nodes should be used, as the number of calculations per node is
proportional to the number of neighbours [@campos_computational_2005].  This is
especially true for the tetrahedral topology, which is the least dense.  That
is, it requires the fewest nodes to fill a volume at any given sampling rate.
Fewer nodes to update means fewer calculations, and a faster overall
simulation.  Lower density meshes also require less storage, so the tetrahedral
scheme is the most memory efficient.

Now, to optimise for accuracy, it now appears that there are two possible
approaches: The first option is to use an interpolated scheme, which is
relatively inefficient in terms of time and space requirements, but which is
most accurate for any given sampling rate.  The second option is to use a
tetrahedral mesh, which is inaccurate but the most time- and space-efficient,
and to oversample until the required accuracy is achieved.  Unfortunately,
there is no prior research comparing the accuracy and efficiency of the
tetrahedral topology against interpolated schemes.  Due to time constraints,
this could not be investigated as part of the Wayverb project (implementation
and numerical analysis of mesh topologies is not trivial).  It was, however,
noted that the tetrahedral mesh is the more flexible of the two approaches.
That is, when accuracy is not important, the tetrahedral mesh will always be
most efficient, but it can be made more accurate by oversampling.  On the other
hand, the interpolated schemes cannot be tuned to produce less accurate results
quickly - they will always be accurate but inefficient.  For these reasons, the
tetrahedral mesh was initially chosen for use in Wayverb.

The tetrahedral mesh was implemented during within the first two months of the
project, with support for microphone modelling.  When it came to implementing
frequency dependent boundary conditions, no prior research could be found
discussing boundary implementations in a tetrahedral topology.  As noted in the
conclusion of [@kowalczyk_room_2011]:

> One aspect that has not been dealt with in this paper, and could be of
interest for future research, is the comparison of the identified schemes with
the tetrahedral topology... However, the applicability of the tetrahedral
stencil to room acoustic simulations is debatable due to the nontrivial
formulation of boundary conditions for complex room shapes.

The design of a new boundary formulation is outside the scope of this research,
which was primarily concerned with the implementation of existing techniques
rather than the derivation of new ones.  Instead, the standard leapfrog
(rectilinear) scheme was used.  Most of the tetrahedral code had to be
rewritten, as the update schemes are very different (tetrahedral nodes have two
possible update equations depending on node orientation), and the memory layout
and indexing methods are more involved.  The new scheme is much simpler than
the tetrahedral mesh, and was quick to implement.

The rectilinear mesh uses the same cubic tessellation as the more complex
topologies mentioned earlier, so in the future the update equation could
conceivably be replaced with a more accurate "interpolated" alternative.  Such
a scheme would yield better computational efficiency for simulations where high
accuracy is required.  The conversion would only require changes to the update
equations (and would complicate the boundary modelling code), but would be more
straightforward than the move from the tetrahedral to the rectilinear topology.
Due to time constraints, this was not possible during the project, meaning that
the waveguide in Wayverb is not the most optimal in terms of accuracy *or*
speed - instead, it was optimised for ease and speed of implementation.  Use of
a more suitable mesh topology would be a sensible starting point for future
development work on the project.

### Source Excitation Method

The question of input and output in the digital waveguide mesh is superficially
simple.  The waveguide mesh is a physical model, and so it is easy to draw an
analogy between the waveguide process, and the process of recording a physical
impulse response.  For physical spaces, a speaker plays a signal at some
location within the space, and a microphone reads the change in air pressure
over time at another point in the space.  The impulse response is found by
deconvolving the input signal from the recorded signal.  The analogue of this
process within the waveguide mesh is to excite the mesh at one node, and to
record the pressure at some output node at each time step, which is then
deconvolved as before.  Recording node pressures is simple, and deconvolution
is a well-known technique.  Injecting a source signal, however, requires
careful engineering in order to maintain the physical plausibility and
numerical robustness of the model.  Source design has two main concerns: the
method that is used to add the signal into the mesh, and the signal that is
injected.

#### Input Node Update Method

Firstly, the input signal may be injected at a single node, or distributed
across several [@jeong_source_2012].  It is complicated to calculate an
appropriate distributed signal [@sakamoto_phase-error_2007], and so single-node
sources are more common in the literature [@jeong_source_2012; @lam_time_2012;
@sheaffer_physical_2014; @murphy_source_2014; @dimitrijevic_optimization_2015].

There are two main options for updating the source node with the input signal.
The first method, known as a *hard source*, simply overwrites the pressure
value at the source node with that of the input signal.  Hard sources are
simple to implement, and ideally couple the input signal to the mesh, but also
scatter any incident wave-fronts [@schneider_implementation_1998].  Although
the initially radiated field is optimal, this spurious scattering is an
undesirable artefact.  Furthermore, the abrupt pressure discontinuity
introduced by the hard source can cause more artefacts through the accumulation
of numerical errors.  For these reasons, hard sources are generally unsuitable
for high-accuracy simulations.

The second method, the *soft source*, instead adds or superimposes the input
signal on the standard mesh update equation.  Soft sources obey the mesh update
equations, and so do not suffer from the scattering problem.  However, they do
not couple the input signal to the mesh, so that the radiated wave front does
not resemble the input signal.  For input signals with DC components, soft
sources can lead to *solution-growth*, in which the DC level of the entire
simulation increases exponentially over time.  This is not to do with numerical
error or stability.  Rather, the DC component suggests that the input signal is
of infinite length, and that the source continues to create volume velocity for
the duration of the simulation [@sheaffer_physically-constrained_2012].  That
is, the model is valid, but it is being used to model a physically-implausible
situation.  Solution-growth is unacceptable for modelling purposes.  In all
cases it creates an increasing DC component which is difficult to remove from
the output.  It also leads to a loss of precision in floating-point
simulations, as intervals between floating-point numbers are larger for
higher-magnitude numbers.  The low-magnitude content of interest cannot be
specified with high precision when it is superimposed on a high-magnitude DC
component.  In the worst case, the DC component will build up so much that the
numerical representation "overflows", placing an upper bound on the duration of
the simulation.  Some of the drawbacks of the soft source can be alleviated by
carefully constructing the input signal, which will be discussed later.

Solution growth is not seen in hard source models, because the source node
pressure is replaced by that of the input function.  Therefore, at the source
node, there is no addition of DC level.  Unfortunately, rarefaction cannot
occur at the source node, which can cause low-frequency oscillations instead.
These oscillations are easier to remove than an increasing DC offset, and have
less impact on precision.

A special case of the soft source is the *transparent source*, described in
[@schneider_implementation_1998].  This method is the same as the soft source,
but additionally subtracts the convolution of the mesh impulse response with
the input signal from the input node.  The resulting wave-front has the same
characteristics as that produced by a hard source, with the additional benefit
that incident wave-fronts are not scattered.  The drawback of this technique is
that the mesh impulse-response must be precomputed before the simulation can
begin, which is achieved by running a separate (costly) waveguide mesh
simulation.  Once found, the mesh response can be used for any other simulation
sharing the same mesh topology.  Although the transparent source is complex to
set up, its characteristics seem perfect.  It behaves as if the input signal is
perfectly coupled to the mesh, and there is no scattering problem.  However, it
retains the solution-growth issue of the general soft source.

The hard and soft source methods can be combined, in order to benefit from the
characteristics of both methods.  A *time-limited pulse* method is introduced
in [@jeong_source_2012], in which the source node starts as a hard source, and
reverts to a soft source after a certain period of time has elapsed.  The input
signal is ideally coupled to the mesh for its duration, but thereafter the mesh
can update as usual.  As long as the source is positioned away from reflective
boundaries, this solves the scattering issue.  However, if the source is near
to a reflective boundary, reflected wave-fronts might reach the source node
before it has reverted to the standard update equation, scattering them.  This
input scheme has similar performance to the transparent source, with much
reduced complexity, but it also shares a major drawback.  If the input signal
has a DC component, the time-limited hard source can still cause
solution-growth [@sheaffer_physically-constrained_2012].  In addition, if the
nodes next to the source are not zero when the update equation switches from
hard to soft, the pressure discontinuity introduced may introduce error which
will propagate for the remainder of the simulation [@sheaffer_physical_2014].

On balance, it seems as though the transparent source is the optimum input
method.  Once the mesh impulse response has been computed, the update method
itself is simple to implement, and does not carry any performance penalties.
The only disadvantage is that the input function must be carefully designed to
have no DC component, to avoid solution-growth.  Unfortunately, overcoming this
weakness is somewhat difficult, as will be shown in the following subsection.

#### Input Signal

Of course, the input signal must be bounded in time, so that the simulation can
end.  The shorter the input signal, the fewer simulation steps must be
computed, so shorter inputs are more efficient.  If a single-sample Dirac-delta
is used, all frequencies in the output bandwidth will be excited equally, and
the output will not require deconvolution, which is another computational
saving.  It is plain that a short impulsive input is desirable in terms of
efficiency.  Unfortunately, impulsive signals have a DC component, which can
cause solution-growth with soft, particularly transparent, sources.

The constraints of the input signal are, therefore, as follows: The signal
shall have no DC component.  It shall be as short as possible, to minimise the
simulation time.  It shall have a wide and flat passband, to increase the
accuracy of the deconvolution.  The time-compactness and bandwidth constraints
are mutually exclusive, so in practice the input length must be chosen to
balance bandwidth and calculation time.

Particular constraints of source signals are presented in greater detail in
[@sheaffer_physical_2014], which puts forward an additional constraint, known
as the *differentiation constraint*: The input signal shall be equal to the
first time derivative of fluid emergence.  Fluid emergence should start and end
at zero, which in turn enforces a null DC component.

Some particular possibilities for the input signal are the *sine-modulated
Gaussian pulse*, the *differentiated Gaussian pulse*, and the *Ricker wavelet*.
All of these signals satisfy the differentiation constraint and the length
constraint.  However, they all have non-flat pass-bands, as shown in the
following figure \text{(\ref{fig:input_signal_info})}.  A final option is the
*physically constrained source* (PCS) model presented in
[@sheaffer_physical_2014].  This method can be used to create input signals
with pass-bands much flatter than those of the more conventional pulse and
wavelet signals.

TODO frequency response of inputs

A PCS input signal, combined with the transparent source input method, seems
perfect for this application.  It obeys the differentiation constraint, has a
wide and flat passband, and is short in time.  The soft source will not cause
scattering artefacts, and as the input signal has no DC component, it should
not introduce solution-growth.

A test was devised to ensure that the source injection method did not cause
solution-growth.  A standard rectilinear waveguide mesh with a sampling
frequency of 10KHz was set up within a cuboid room, measuring $5.56 \times 3.97
\times 2.81$ metres. A source was placed at (4.8, 2.18, 2.12), and a receiver
at (4.7, 2.08, 2.02).  The walls of the room were set to have a uniform
broadband absorption of 0.006 (see the [Boundary Modelling]({{ site.baseurl}}{%
link boundary.md %}) section).  "Transparent" input signals were created from a
differentiated Gaussian pulse, a sine-modulated Gaussian pulse, and a Ricker
wavelet, all of which were set to a centre frequency of 0.05 $f_s$.  A
physically-constrained source signal was also generated, using parameters
suggested in section V of [@sheaffer_physical_2014]: a max-flat
finite-impulse-response pulse-shaping filter kernel with 16 taps and centre
frequency of 0.075 $f_s$ and magnitude 250μN was generated; it was passed
through a mechanical shaping filter with radius 5cm, mass 25g, lower cutoff
100Hz, and resonance 0.7; then this signal was passed through an
infinite-impulse-response *injection filter*, and used as a soft source.
(These parameters are reproduced here to ensure that the test is repeatable,
but a full discussion of their meaning is beyond the scope of this paper. The
interested reader is directed to [@sheaffer_physical_2014] or to the
implementation of the physically-constrained source in the Wayverb repository.)
Finally, the simulation was run for around 85000 steps (less than the expected
Sabine RT60 of the room) with each of the four sources, and the response at the
receiver was recorded.

The results of the experiment are shown in the following figure
\text{(\ref{fig:solution_growth_results})}.  The response of a transparent
Dirac source (which has a strong DC component) is also shown.  All the sources
with no DC component show significantly less solution-growth than the
transparent Dirac source.  However, they *do* all show the effects of
solution-growth.

TODO solution growth results

The solution-growth seen here only becomes prominent towards the end of the
simulation, after around 60000 steps.  However, papers which propose
countermeasures to the solution-growth problem generally only test their
solutions up to 15000 steps or so [@sheaffer_physical_2014;
@sheaffer_physically-constrained_2012; @jeong_source_2012].  The results of
testing the solution in [@dimitrijevic_optimization_2015] are not even
presented.  It is entirely possible that these input methods have not been
tested in such a long simulation before.

The reason for the solution-growth is not clear. In general, the problem is
caused by repeated superposition of the DC level, which is reflected from
boundaries.  The waveguide has no inherent way of removing DC, so *any* small
DC component will accumulate over time.  If the original signal does not have a
DC component, then the DC is being added from elsewhere.  The most likely
origin is numerical error in the waveguide mesh.  The experiment above uses
32-bit single-precision floating-point to represent the pressure of mesh nodes,
which is necessary because using double-precision would double the memory usage
and halve the computational throughput.  It is possible that error in these
single-precision calculations manifests as a tiny DC component, which then
multiplies as the simulation progresses.

Whatever the reason, it is clear that using a soft source generally causes a DC
offset to accumulate, even when the input signal has no DC component.  Soft
sources may be suitable for shorter simulations, however, without running the
simulation, it is impossible to know how much DC will accumulate, and whether
or not it will remain within acceptable bounds.  Therefore, in general, current
forms of the soft source are not appropriate for arbitrary room simulation.

As an alternative to a soft source, Wayverb currently uses a hard source with a
Dirac impulse as its input method.  Though this still causes low-frequency
error [@sheaffer_physical_2014], this error manifests as an oscillation rather
than an exponential growth. The oscillation tends to be below the audible
range, and therefore can be removed without affecting the perceived quality of
the simulation. This removal is achieved with no phase modifications by
transforming the signal into the frequency domain, smoothly attenuating the
lowest frequencies, and then converting back to the time domain.
The main drawback of the hard source is its scattering characteristic.
Though undesirable, this behaviour has a physical analogue: in a
physical recording, reflected wave-fronts would be scattered from the speaker
cabinet or starter pistol used to excite the space.

Code for modelling soft sources remains in the Wayverb repository, so if future
research uncovers a soft source without solution-growth, it will be easy to
replace the current source model with an improved one.

## Implementation

Here, the final waveguide, as implemented in Wayverb, is described.

TODO mesh setup - nodes inside / outside, storage method TODO sample rate,
update equations etc. Courant number

TODO mention boundary setup?
