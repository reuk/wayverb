---
layout: page
title: Boundary modelling
navigation_weight: 7
---

---
reference-section-title: References
...

# Boundary Modelling {.major}

## Introduction

The ideal boundary model would allow complete control over the frequency- and direction-dependent absorption and scattering of a surface.
Though this is reasonably straightforward in geometric models, it is far from a solved problem for the digital waveguide mesh (DWM).
Several possible implementations are discussed in the literature, each with unique drawbacks.

This section will begin by discussing the ideal behaviour of modelled acoustic boundaries.
Then, the implementation for geometric models will be discussed.
Possibilities for DWM boundary models will be investigated, and the final choice of method explained.
The geometric and DWM implementations will be evaluated and compared, to ensure equivalence.

## Background

The books by Vorlander [@vorlander_auralization:_2007, p. 35] and Kuttruff [@kuttruff_room_2009, p. 35] both devote entire chapters to the topic of sound reflection and scattering.
Please refer to these books for a detailed and broad explanation of reflection effects.
To avoid unnecessary duplication, this background will be brief, aiming only to put terminology and decisions in context.

<!--
TODO talk about plane waves vs spherical waves
In acoustic simulation, incident waves are generally considered to be plane waves.
This simplifies most calculations, but the source of the wave must not be far enough from any reflecting surface that the curvature of the wave-front is negligible.
-->

### Magnitude and Phase

The reflection factor $R$ is a complex value given by $R=|R|\exp(i\chi )$, which describes a modification to the amplitude and phase of the reflected wave ($|R|$ is the magnitude term, $\chi$ is phase).
This factor depends both on the frequency and direction of the incident wave.
When $\chi = \pi$, $R=-1$, corresponding to a phase reversal.
This is known as a "soft" wall, but is rarely seen in room acoustics.
It is reasonable to assume that reflections are in-phase in the majority of problems.

The wall impedance $Z$ is defined as the ratio of sound pressure to the normal component of particle velocity at the wall surface.
It is related to the reflection factor by $R=\frac{Z\cos\theta-Z_0}{Z\cos\theta+Z_0}$, where $\theta$ is the angle of incidence, and $Z_0$ is the characteristic impedance of the propagation medium, normally air.
In the case that the wall impedance is independent of the wave angle-of-incidence, the surface is known as *locally reacting*.
A locally reacting surface does not transmit waves tangentially along the wall surface.

The absorption coefficient $\alpha$ of the wall is related to the reflection factor by the equation $\alpha =1-|R|^2$.
It describes the proportion of incident energy lost during reflection.

Properties of surfaces in an acoustic simulation may be described fully by either the reflection factor or impedance.
The absorption coefficient fails to encode the phase-change properties of the surface, and so cannot fully describe the full range of possible characteristics.
However, if surfaces are assumed to be "hard", i.e. they do not induce phase changes, then the absorption coefficient is an adequate descriptor.

### Scattering

The reflection factor, absorption coefficient, and wall impedance describe the behaviour of perfectly-reflected (specular) waves.
If the reflecting surface has imperfections or details of the same order as the wavelength, as many surfaces in the real world do, then some components of the reflected wave will be *scattered* instead of specularly reflected.

Describing the nature of the scattered sound is more complicated than specular reflections.
A common method is to use a *scattering coefficient*, $s$, which describes the proportion of outgoing energy which is scattered, and which may be dependent on frequency \text{(see figure \ref{fig:scattering})}:

$$E_{\text{scattered}}=E_{\text{incident}}(1-\alpha)s, E_{\text{specular}}=E_{\text{incident}}(1-\alpha)(1-s), E_{\text{total}}=E_{\text{incident}}(1-\alpha)$$

![Reflected components from a rough surface.\label{fig:scattering}](images/scattering)

Alone, the scattering coefficient fails to describe the directional distribution of scattered energy.
In the case of an ideally-diffusing surface, the scattered energy is distributed according to Lambert's cosine law.
That is, the intensity depends only on the cosine of the outgoing scattering angle, and is independent of the angle of incidence \text{(see figure \ref{fig:lambert})}.
More complex scattering distributions, which also depend on the outgoing direction, are possible [@christensen_new_2005; @durany_analytical_2015], but there is no single definitive model to describe physically-accurate scattering.

![Lambert scattering. Scattered intensity is independent of incident angle.\label{fig:lambert}](images/lambert)

## Geometric Implementation

The geometric implementation of reflective surfaces is straightforward.
In both image-source and ray tracing methods, each ray starts with a certain intensity or pressure.
For specular reflections, the ray pressure must merely be multiplied by the wall reflection coefficient, which may derived from the absorption and scattering coefficients using the equations above.
Specifically, the normal-incidence specific impedance $\xi_0$ is calculated using $\xi_0=\frac{1+R_0}{1-R_0}$ where $R_0$ is the normal-incidence reflection factor.
Then, the angle-dependent reflection factor is given by $R_\theta=\frac{\xi_0\cos\theta-1}{\xi_0\cos\theta+1}$ where $\theta$ is the angle of incidence [@southern_room_2013].

If the reflection factor is dependent on frequency then the frequency range, and the pressure carried by the ray, must be discretised into bands.
Then each band must be modified using a representative reflection factor for that frequency range.
This is similar to the approach taken in graphical ray tracing, in which each ray carries separate red, green, and blue components.
These components are modified independently, depending on the colour of the reflective surface.

By definition, image-source models find only specular reflections (i.e. image sources), so scattering is not implemented in these models.
Scattering can be implemented in ray tracers, but there is no consensus on the optimum method.
One option is to spawn two rays at every reflection: a specular ray, and a diffuse ray with random direction.
Though this properly replicates the theory, it leads to an explosion in the number of rays which must be traced, so is impractical in most cases.
A second option is to decide, using the scattering coefficient as a probability, whether the reflection should be specular or diffuse [@savioja_overview_2015].
This solves the ray-explosion problem, but requires an additional random number to be generated per-reflection, which can be costly for large numbers of rays.
An elegant solution is to simply mix the specular and diffuse rays together, using the scattering coefficient as a weighting [@rindel_use_2000], a technique known as *vector based scattering* [@christensen_new_2005].
This is the approach taken by Wayverb.
A major drawback of all these scattering methods is that the scattering coefficient can only be frequency-dependent if a separate ray is traced for each band.
If a single ray is used to carry all frequency components, then each component must be scattered in exactly the same way.

The plain scattering model affects only the ongoing ray direction and amplitude.
However, it is worth considering that, at each reflection, the scattered energy may be directly visible to the receiver.
This fact is exploited by the *diffuse rain* technique, in which each reflection is considered to spawn a "secondary source" which emits scattered energy towards the receiver.
This scattered energy is recorded only if the secondary source is visible from the receiver.

Assuming perfect Lambert diffusion, the magnitude of diffuse rain scattered energy is given by [@schroder_physically_2011, p. 64]:

$$E_{\text{scattered}} = E_{\text{incident}}(1-\alpha)2s\cos\theta(1-\cos\frac{\gamma}{2})$$

Here, $\theta$ is the angle from secondary source to receiver relative against the surface normal, and $\gamma$ is the opening angle \text{(shown in figure \ref{fig:diffuse_rain})}.
The magnitude of the scattered energy depends on the direction from the secondary source to the receiver (by Lambert's cosine law), and also on the solid angle covered by the receiver.

![Angles used in the diffuse rain equation for a spherical receiver.\label{fig:diffuse_rain}](images/diffuse_rain)

## DWM Implementation

Modelling boundary surfaces in the digital waveguide mesh (DWM) is a problem that is yet to be solved in a way that is flexible, accurate, and realistic.
Methods from the literature each have unique drawbacks, meaning none are particularly satisfactory for applications where realism is required.
The two most common methods will be reviewed, and the choice of method for Wayverb will be explained.

### Possible Methods

#### KW-Pipe Technique

This method is described in [@murphy_kw-boundary_2007] and [@kelloniemi_frequency-dependent_2006].

There are two main technically-equivalent formulations of digital waveguides meshes, known as *W-models* and *K-models*.
W-models are based on travelling wave variables, which allow for straightforward interaction with a variety of termination types, such as rimguides, fractional delays, or wave digital filters.
Wave digital filters, in particular, could be used to model frequency-dependent boundaries and air absorption.
However, W-models have great memory requirements, making them impractical for large multi-dimensional simulations.
K-models are based on Kirchhoff variables, and depend on physical quantities rather than travelling wave components.
Under certain conditions, K-models and finite-difference time-domain (FDTD) simulations are equivalent.
FDTD models have much smaller memory requirements than W-models, at the cost of decreased flexibility of filtering, as these models cannot directly interact with wave digital filters.

The KW-pipe is a "converter" between wave- and Kirchhoff- variables, which is designed to allow the majority of a model (that is, the air-filled space inside it) to be constructed as a K-model waveguide mesh.
At the boundaries of the model, the KW-pipe is used to connect K-model nodes to W-model nodes.
These W-model nodes can then be connected to wave digital filters to simulate frequency-dependent absorption of wave energy.
The complete model retains both the memory-efficiency of the K-model and the termination flexibility of the W-model, with the drawback of additional implementation complexity at the interface between the two model types.

This sounds extremely promising, but has a major drawback, as described in [@kowalczyk_modeling_2008]:
while the inside of the mesh will be 2- or 3-dimensional, the boundary termination afforded by the wave-variable boundary is 1-dimensional.
Each boundary node connects to just the closest interior node.
As a result, the edges and corners are not considered to be part of the model, as these nodes do not have a directly adjacent interior node.
Additionally, the 1D boundary termination equation implies a smaller inter-nodal distance than that of the 2D or 3D mesh interior.
This means that when updating an interior node next to a boundary, the inter-nodal distance is greater than when updating the boundary node itself.
For these reasons, the 1D termination is unphysical and can lead to large errors in the phase and amplitude of reflections.

#### Locally Reactive Surfaces Technique

This method, described in [@kowalczyk_modeling_2008], aims to create physically correct higher-dimensional boundaries by combining a boundary condition, defined by a boundary impedance, with the multidimensional wave equation.
This leads to a model for a *locally reacting surface* (LRS), in which boundary impedance is represented by an infinite-impulse-response (IIR) filter.

As noted above, a surface is locally reacting if the normal component of the particle velocity on the boundary surface is dependent solely upon the sound pressure in front of the boundary.
In most physical surfaces, the velocity at the surface boundary will also be influenced by the velocity of adjacent elements on the boundary, so LRS is not a realistic physical model in the vast majority of cases.

However, despite that it is not a realistic physical model, the implementation of the LRS modelling technique is both stable and accurate, as opposed to the 1D KW-pipe termination, which does not accurately model even locally-reacting surfaces.

The LRS model leads to an implementation that is efficient (as it is based completely on the K-model/FDTD formulation) and tunable (boundaries are defined by arbitrary IIR filters).

### Choice of Boundary Technique for the DWM

The LRS technique was chosen, as it represented the best compromise between memory efficiency, customization and tuning, and realism.
The particular strengths of this model are its performance and tunability, though as mentioned previously it is not physically accurate in many cases.
That being said, neither of the boundary models considered are particularly realistic, so even for applications where realism is the most important consideration, the LRS model seems to be the most appropriate.

### LRS Implementation

See [@kowalczyk_modeling_2008] and [@kowalczyk_modelling_2008] for a more detailed explanation.

In the [Background] section it was noted that the reflection characteristics of a surface are fully defined by a reflection coefficient or wall impedance, both of which might be frequency- and direction-dependent.
The LRS technique starts from the definition of wall reflectance:

$$R=\frac{Z\cos\theta-Z_0}{Z\cos\theta+Z_0}$$

This may instead be written in terms of *specific acoustic impedance* $\xi$, which is equal to the wall impedance $Z$ divided by the acoustic impedance of the propagation medium (air) $Z_0$: $\xi=\frac{Z}{Z_0}$.

$$R=\frac{\xi\cos\theta-1}{\xi\cos\theta+1}$$

This equation can be rearranged to define the wall impedance in terms of the reflection coefficient.
Further, the reflection coefficient can be replaced with a digital filter $R_0(z)$, describing the frequency-dependent normal-incidence behaviour of the surface.
This filter might be derived from per-band absorption coefficients, using the relationship $|R|=\sqrt{1-\alpha}$.
In Wayverb, reflection magnitudes are found in this way, and then the Yule-Walker method is used to approximate coefficients for $R_0$.
The substitution leads to an equation for a filter, describing the specific acoustic impedance of the surface:

$$\xi(z)=\frac{1+R_0(z)}{1-R_0(z)}$$

This impedance filter is most efficiently implemented as an *infinte impulse response* (IIR) filter, though surfaces with detailed frequency responses will require high-order filters, which tend to become numerically unstable.
The usual solution to this problem would be to split the high-order filter into a series-combination of lower-order filters, however the LRS requires access to intermediate values from the filter delay-line which makes this approach impossible.
An alternative solution is suggested in [@oxnard_frequency-dependent_2015], which suggests running the entire simulation multiple times, once for each octave band.
This means that the boundary filters can be single-order, and resistant to accumulated numerical error.
Compared to high-order boundary filters, this method gives much improved accuracy, but at the (immense) cost of running the entire simulation multiple times.
In Wayverb, both approaches are taken, allowing the user to choose between a fast, inaccurate single-run simulation with high-order filters; or a slow, accurate multi-run simulation with low-order filters.

The implementation of the boundaries themselves in the DWM is fairly simple.
Appropriate impedance filter coefficients can be inserted into special update equations, which are found by combining the discrete 3D wave equation with the discrete LRS boundary condition.
Recall that the DWM operates by updating each mesh node individually, depending on the states of the immediately adjacent nodes.
To model boundaries, the update equation for each of the boundary nodes is replaced.

In the case of a flat wall, the boundary node is adjacent to a single inner-node, and a "1D" update equation is used.
Where two perpendicular walls meet, the nodes along the edge will each be adjacent to two "1D" nodes, and a "2D" update equation is used for these nodes.
Where three walls meet, the corner node will be directly adjacent to three "2D" nodes, and a "3D" update equation is used for this node.
The three types of boundary nodes are shown in the following diagram \text{(\ref{fig:boundary_type_diagram})}.
Note that this method is only capable of modelling mesh-aligned surfaces.
Other sloping or curved surfaces must be approximated as a group of narrow mesh-aligned surfaces separated by "steps".
For example, a wall tilted at 45 degrees to the mesh axes will be approximated as a staircase-like series of "2D" edge nodes.

![The three types of boundary nodes, used to model reflective planes, edges, and corners. 1D nodes are adjacent to inner nodes, 2D nodes are adjacent to two 1D nodes, and 3D nodes are adjacent to three 2D nodes.\label{fig:boundary_type_diagram}](images/boundary_diagram)

<!--
TODO code discussion?

#### Code

No papers give any hints about how to structure the simulation data in memory.
In Wayverb, the metadata for each node is stored like so (actual node pressures are stored separately):

    struct condensed_node final {
		int boundary_type;
		unsighed int boundary_index;
    };

The `boundary_type` field is a bitfield which encodes the update equation that should be used for the node, by `or`ing together fields from an enum:
		
	typedef enum {
		id_none = 0,
		id_inside = 1 << 0,
		id_nx = 1 << 1,
		id_px = 1 << 2,
		id_ny = 1 << 3,
		id_py = 1 << 4,
		id_nz = 1 << 5,
		id_pz = 1 << 6,
		id_reentrant = 1 << 7,
	} boundary_type;

If no bits are set, the node is completely outside the simulation.
If the least significant bit (LSB) is set, the node is an inner node, and should use the air update equation.
Otherwise, the node is a boundary node, and should use a boundary update equation depending on the number of set bits (1 bit set = 1D update equation, 2 bits set = 2D update etc.).

The `boundary_index` field is an index into an array of filter state structs.
As only the bottom 8 bits of the `boundary_type` field are used, this field could be stored in the top 24 bits, to halve the memory required for each struct.
This approach will be taken in a future revision of the software.

TODO diagram of memory map/layout

-->

## Test Procedure

Only the LRS method is tested here.
The implementation of frequency-dependent boundaries in geometric simulations amounts to multiplication by a reflection coefficient (with optional scattering) per-band, so there is little to test.
The LRS waveguide boundary is more complicated, as it embeds IIR filters into the waveguide boundaries, so it is worth testing that the boundary nodes behave as expected.

The testing procedure used is similar to that in [@kowalczyk_modeling_2008].
Code for the test can be seen at <https://github.com/reuk/wayverb/blob/master/bin/boundary_test/boundary_test.cpp>.

### Simulation Parameters

* Cubic room, $300 \times 300 \times 300$ nodes.
* 8 KHz mesh sampling frequency.
* Run for 420 steps.
* Source and receiver placed 37 node-spacings from the centre of the boundary.

The following diagram \text{(\ref{fig:boundary_test_setup})} shows the testing setup, which will be explained in the next section.

![The setup of the two room-sizes, and the positions of sources and receivers inside.\label{fig:boundary_test_setup}](images/boundary_testing_setup)

### Method

A simulation was run using the parameters above, and the reflected signal at the
receiver was recorded.
This first recording, $r_f$, contained a direct and a reflected response.
Then, the room was doubled in size along the plane of the wall being tested,
essentially removing this boundary from the model.
The simulation was run again, recording just the direct response at the receiver
($r_d$).
Finally, the receiver position was moved to its reflected position "through" the
tested wall, and the simulation was run once more, producing a free-field
response ($r_i$).

The reflected response was isolated by subtracting $r_d$ from $r_f$, cancelling
out the direct response.
This isolated reflection is $r_r$.
To find the effect of the frequency-dependent boundary, the frequency content of
the reflected response was compared to the free-field response $r_i$.
This was achieved by windowing $r_r$ and $r_i$ with the right half of a Hanning
window, then taking FFTs of each.
The experimentally determined numerical reflectance was determined by dividing
the absolute values of the two FFTs.

To find the accuracy of the boundary model, the numerical reflectance was
compared to the theoretical reflection of the digital impedance filter being
tested, which is defined as:

$$R_{\theta, \phi}(z) = \frac{\xi(z)\cos\theta\cos\phi - 1}{\xi(z)\cos\theta\cos\phi + 1}$$

where $\theta$ and $\phi$ are the reflection azimuth and elevation respectively.

The test was run for three different angles of incidence, with azimuth and elevation of 0, 30, and 60 degrees respectively.
Three different sets of surface absorption coefficients were used, giving a total of nine combinations of source position and absorption coefficients.
The specific absorption coefficients are those suggested in [@oxnard_frequency-dependent_2015], shown in the following table:

band centre frequency / Hz  31      73      173     411     974     
--------------------------- ------- ------- ------- ------- ------- 
plaster                     0.08    0.08    0.2     0.5     0.4     
wood                        0.15    0.15    0.11    0.1     0.07    
concrete                    0.02    0.02    0.03    0.03    0.03    

The boundary filter for each material was generated by converting the absorption coefficients to per-band reflectance coefficients using the relationship $R=\sqrt{1-\alpha}$.
Then, the Yule-Walker method from the ITPP library [@_itpp_2013] was used to calculate coefficients for a sixth-order IIR filter which approximated the per-band reflectance.
This filter was converted to an impedance filter by $\xi(z)=\frac{1+R_0(z)}{1-R_0(z)}$, which was then used in the boundary update equations for the DWM.

## Results

The results are shown in the following figure \text{(\ref{fig:reflectance})}.
Although the waveguide mesh has a theoretical upper frequency limit of 0.25 of
the mesh sampling rate, the 3D FDTD scheme has a cutoff frequency of 0.196
of the mesh sampling rate for axial directions.
This point has been marked as a vertical line on the result graphs.

![Measured boundary reflectance is compared against the predicted reflectance, for three different materials and three different angles of incidence.\label{fig:reflectance}](images/reflectance)

## Evaluation

The initial impression of the results is that the measured response is reasonably accurate, to within 6dB of the predicted response, but only below 0.15 of the mesh sampling rate.
Above this point, the responses become very erratic, with very large peaks and troughs.
This is especially true for the on-axis (0-degree) tests, where some erratic behaviour is seen as low as 0.12 of the mesh sampling rate.
This may be due to numerical dispersion in the waveguide mesh, which is greatest along axial directions [@kowalczyk_modeling_2008].
At the other end of the spectrum, the results look acceptable, adhering closely to the predicted response.

The poor performance at the top of the spectrum is not particularly concerning, as the waveguide mesh is designed to generate low-frequency content.
If wideband results are required, then the mesh can simply be oversampled.
To prevent boundary modelling error affecting the results of impulse response synthesis in the Wayverb app, the mesh cutoff frequency is locked to a maximum of 0.15 of the mesh sampling rate.

It is also worth noting that ideally this experiment would be conducted with a
completely flat wave-front, which is not easily accomplished.
The experiments in [@kowalczyk_modeling_2008] use large meshes (around 3000 by 3000
nodes, nine million in total) and place the sources a great distance away from
the boundary being studied in order to maintain a mostly-flat wave-front.
However, the experiments are only run in two dimensions.
In fact, no experimental results are given for the implementation of three-dimensional boundaries.
This is probably because running a 3D simulation on a similar scale would require
a mesh of twenty-seven billion nodes, which in turn would require gigabytes of
memory and hours of simulation time.

According to [@kowalczyk_modeling_2008], in some of the experiments with 2D meshes,
there are disparities at low frequencies between the predicted and actual
results, which they say is an artefact of non-flat wave-fronts.
Interestingly, there is little low-frequency error in the experimental results above, despite the fact that the source is placed very close to the boundary, and the wave-front is therefore very rounded.
This might, however, be the cause of the relatively small broadband fluctuations between 0 and 0.15 of the mesh sampling rate.
The filters used in this test are also of much higher order than those tested in [@kowalczyk_modeling_2008], giving a greater chance of accumulated numerical error.
This may be the cause of the volatile high-frequency behaviour.

In conclusion, for the most part, the results presented adhere closely to the expected results, with the caveat that the surface reflectance is only accurate at low frequencies, below around 0.15 of the mesh sampling rate.
Different absorption coefficients lead to clearly-different reflectance coefficients, which are additionally accurate at multiple angles of incidence.
While not completely accurate, this model is both fast and tunable, making it a good candidate for boundary modelling in room acoustics simulations.

