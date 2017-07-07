---
layout: page
title: Image-source Model
navigation_weight: 3
---

---
reference-section-title: References
...

# Image-source Model {.major}

## Background

### Basic Method

The image-source method aims to find the purely specular reflection paths
between a source and a receiver.  This process is simplified by assuming that
sound propagates only along straight lines or rays.  Sound energy travels at a
fixed speed, corresponding to the speed of sound, along these rays.  The energy
in each ray decreases with $1/r^2$, where $r$ is the total distance that the
ray has travelled [@vorlander_auralization:_2007, p. 58].

Rays are perfectly reflected at boundaries.  When a ray is reflected, it spawns
a secondary source "behind" the boundary surface.  This source is located on a
line perpendicular to the wall, at the same distance from it as the original
source, as if the original source has been "mirrored" in the surface (an
example is shown in +@fig:image_source_construction).  This new "image" source
now represents a perfect reflection path, in that the distance along the
straight line between the receiver and the image source has the same length as
the path from the *real* source to the receiver, reflected in the boundary. If
the source is reflected in a single boundary, this represents a first-order
reflection.  A ray which is reflected from several boundaries is represented by
a "higher-order" image-source, which has been mirrored in each of those
boundaries in turn [@kuttruff_room_2009, p. 104].

![Image sources are found by reflecting the source position in a
boundary.](images/image_source_construction){#fig:image_source_construction}

All sources, original and image, emit the same impulsive source signal at the
same time.  The total impulse response (i.e. sound pressure against time) is
found by summing the signals from each source, delayed and attenuated
appropriately depending on the distance between that source and the receiver,
which is equivalent to the length of the specular reflection path.  The
frequency response of the signal from each image source will additionally be
modified depending on the characteristics of each boundary in which that source
was reflected.

In the real world, not all energy is perfectly reflected at a boundary.  Some
energy will be randomly diffused in non-specular directions.  The image-source
model is not capable of modelling this phenomenon, though this is not
particularly problematic.  Consider that, once scattered, sound energy cannot
become un-scattered. The conversion from incoming energy to scattered energy is
unidirectional, so repeated reflections cause the ratio of scattered to
specular energy to increase monotonically.  Kuttruff shows that, though the
earliest reflections may be largely specular, after a few reflections the large
majority of sound energy becomes diffuse [@kuttruff_room_2009, p. 126].  This
suggests that the image model should be used only for very early reflections,
where most energy is not scattered, and a secondary model used to compute late,
diffuse reflections. In Wayverb, the image model is used for early reflections,
and stochastic ray-tracing is used for the diffuse tail. The combination of the
two models is described in the [Hybrid Model]({{ site.baseurl }}{% link
hybrid.md %}) section.

### Audibility Checking

The position of an image source is found by reflecting it in one or more
surfaces. Next, it must be checked to ensure it represents a valid specular
path to the receiver.  This is known as an *audibility test*
[@vorlander_auralization:_2007, p. 202].

Consider first a source $S$, a receiver $R$, and a single wall $A$.  The source
is reflected in $A$, creating an image-source $S_A$.  A line is constructed
from $R$ to $S_A$.  If this line intersects $A$, then $S_A$ represents a valid
image source.  Otherwise, there is no possible specular reflection involving
$S$, $R$ and $A$.

Now consider two walls, $A$ and $B$.  The image source $S_{AB}$ has been
reflected in $A$ then $B$.  For the image-source to be valid:

* $R \rightarrow S_{AB}$ must intersect $B$ at some point
  $B_\text{intersection}$, 
* $B_\text{intersection} \rightarrow S_A$ must intersect $A$ at
  $A_\text{intersection}$, *and* 
* $A_\text{intersection} \rightarrow S$ must not intersect with any scene
  geometry.

The validation of a third-order image-source will require three intersection
checks, a fourth-order image will require four checks, and so on.  This method
of tracing backwards from the receiver to each of the image sources is known as
*backtracking*. This process is shown in +@fig:backtracking.

![**Left:** The paths $S \rightarrow A \rightarrow R$ and $S \rightarrow A
\rightarrow B \rightarrow R$ are both valid. **Right:** $S \rightarrow B
\rightarrow A \rightarrow R$ is an invalid path because $R \rightarrow S_{BA}$
does not intersect $A$.](images/backtracking){#fig:backtracking}

### Accelerating the Algorithm

The naive method to find all the image sources for a scene is very expensive:
Consider that to find a single first-order image source, the original source
must be mirrored in a surface, and then an intersection test must be conducted
between that surface and the image-source-to-receiver ray.  To find all
first-order image sources, this process must be carried out for all surfaces in
the scene.  To find all second-order image sources, each of those first-order
images must be tested against every surface.  This continues for higher-order
images, so that the number of checks for image sources of a given order is
equal to the number of surfaces raised to the power of that order.  The
relationship between the image-source order and the computation time is
therefore exponential, with average-case complexity of $O(N^o)$ where $N$
denotes the number of boundaries, and $o$ is the image-source order.  As a
result, it is practically impossible to validate all possible image-source
positions when the room geometry is complex or the image-source order is high.
As an example, imagine that a particular (fictional) simulator might take a
second to simulate a scene with 100 surfaces to an image-source depth of 2.  If
the image source depth is increased to facilitate a longer reverb tail,
third-order image sources will take 100 seconds to compute, and fourth-order
sources will take 3 hours. Fifth-order sources will take 12 days.  Clearly, it
is not possible to achieve Wayverb's efficiency goal of "ten minutes or fewer"
under all circumstances using this naive image source technique.

The majority of higher-order image sources found with the naive algorithm will
be invalid.  That is, they will fail the audibility test.  For example, for
tenth-order image-sources in a shoebox-shaped room, there are around 1.46e7
different image sources, only 1560 of which are valid [@kuttruff_room_2009, p.
323].  If the invalid image-sources can be discarded early, without requiring
individual checking, then the amount of computation can be greatly reduced to a
viable level.  As explained above, image sources above order four or five are
rarely required, but even these can be very time-consuming to find with the
naive method.  Optimisations are, therefore, a necessity for all but the
simplest simulations.

To accelerate the image-source process, [@vorlander_auralization:_2007]
suggests tracing a large number of rays in random directions from the source,
and logging the unique paths of rays which eventually intersect with the
receiver. The complexity of ray tracing grows linearly rather than
exponentially with reflection depth, meaning it can find deeper reflections
with far fewer operations than the image-source method. Each unique path found
in this way is used to generate an image source sequence, which is then checked
as normal.  This technique has the advantage that the majority of surface
sequences are *not* checked, so the image-source process is fast.  However, if
the preliminary ray-tracer is not run with enough rays, it is likely to miss
some valid paths, especially in complex scenes.  Additionally, if the receiver
volume is too great, then some invalid paths may still be detected.

The technique used by Wayverb is similar to that presented in
[@vorlander_auralization:_2007], but makes a small change to the acceleration
process. Note that this does not affect the physical interpretation of the
image-source model. It simply changes the way in which ray paths are initially
selected for further audibility checking.

A large number of random rays are traced, as before, but at each reflection
point, the receiver is checked to see whether it is visible.  If it is, then
the surface sequence is checked for a valid image-source. This adds constant
work per-reflection to the ray-tracing process, which is insignificant in terms
of overall time-complexity.  This technique has two main advantages. Firstly,
more paths are checked, so it is more likely to find all the valid
image-sources. Instead of just checking ray paths which intersect the receiver,
this method checks all paths which are *capable* of intersecting the receiver.
Secondly, initial ray paths don't have to be specular, so techniques like
*vector-based scattering* can be used.  The disadvantage is that a greater
number of validity checks are required, though this number is still many times
smaller than would be required by a naive implementation.

## Implementation

Here the concrete implementation of the image-source method is presented, as it
is used in Wayverb. *@fig:image_source_process gives an overview of the entire
process.

![Creation of an impulse response using image
sources.](images/image_source_process){#fig:image_source_process}

<!--
The simulation prerequisites are:

* source position 
* receiver position 
* speed of sound in air 
* acoustic impedance of air 
* a scene, made up of triangles, where each triangle has an associated material
  comprised of multiband absorption and scattering coefficients (note that 
  curved surfaces are not supported, and must be approximated by small 
  triangles)
-->

The main prerequisite for the simulation is a scene, made up of triangles,
where each triangle has an associated material comprised of multi-band
absorption coefficients.  First, an axis-aligned bounding box is computed for
this scene, and split into uniformly sized cuboid *voxels*.  Each voxel holds a
reference to any triangles in the scene which happen to intersect with that
voxel.  The voxel mesh acts as an "acceleration structure", speeding up
intersection tests between rays and triangles.  To check for an intersection
between a ray and a number of triangles, the simplest method is to check the
ray against each triangle individually, which is very time consuming.  The
voxel mesh allows the number of checks to be greatly reduced, by checking only
triangles that are within voxels that the ray intersects.  These voxels can be
found very quickly, by "walking" the voxels along the ray, using an algorithm
presented by Amanatides, Woo et al. [@amanatides_fast_1987].  For large scenes
with many triangles, this method can lead to speed-ups of an order of magnitude
or more.  All ray-intersection tests mentioned throughout this thesis use the
voxel-acceleration method, unless explicitly noted. In particular, both the
initial ray-casting *and* the image-source audibility-checking are accelerated
using the voxel method.

Rays are fired in uniform random directions from the source (item 1 in the
figure).  Each ray is checked for an intersection with the scene, and if an
intersection is found, some data about the intersection is recorded.
Specifically, the record includes the triangle which was intersected, and
whether or not the receiver is visible from the intersection point.
Then, the vector-based scattering method [@christensen_new_2005] (see the [Ray
Tracing]({{ site.baseurl }}{% link ray_tracer.md %}) section for details on
this) is used to find the directions of new rays, which are fired from the
intersection points.  The ray-tracing process continues up to a certain depth,
which is artificially limited to ten reflections in Wayverb.  For most
simulations, three or four reflections should be adequate, though this depends
somewhat on the scattering coefficients of the surfaces. After a few
reflections, most sound energy is diffuse rather than specular, and other
methods are better suited to modelling this scattering, as explained in the
[Basic Method] subsection above.

The ray tracer produces a list of reflection paths for each ray, where a single
reflection path is defined by a sequence of visited surfaces (item 2 in the
figure).  Some rays may follow the same paths (reflect from the same surfaces),
and so duplicate paths must be removed.  This is achieved by condensing per-ray
information into a tree of valid paths (item 3).  Each node in the tree stores
a reference to a triangle in the scene, and whether or not the receiver is
visible from this triangle.  Each unique path starting from a root node in the
tree represents a possible image source contribution, which must be checked.
This checking is carried out using the backtracking method explained above
(item 4). The tree structure can be traversed using depth-first recursion,
allowing the results of some intermediate calculations to be cached between ray
paths, speeding up the calculation with only minimal memory overhead.  This is
similar to the approach mentioned in [@savioja_overview_2015].  Also, because
the tree is immutable, it can be shared between multiple worker threads, which
independently check each branch for valid image sources.  The nature of the
recursive algorithm makes it a poor fit for an OpenCL implementation, so native
(CPU) threads are used instead.

Some paths in the tree may not actually produce valid image sources, and these
paths are discarded (item 5).  For paths which *do* contribute valid image
sources, the propagation delay and frequency-dependent pressure of the image
source signal must be found.  As described by Kuttruff, the propagation delay
is equal to the distance from the receiver to the image source, divided by the
speed of sound, [@kuttruff_room_2009, p. 325].  The pressure content is found
by convolving together the reflectances of all intermediate surfaces.  This is
equivalent to a single multiplication per frequency band, as long as the
reflectance value for each band per surface can be represented by a single real
value.

The surface reflectances are found by converting per-band absorptions into
per-band normal-incidence reflectance magnitudes using

$$|R|=\sqrt{1-\alpha}$$ {#eq:}

where $R$ is the surface reflectance, and $\alpha$ is the absorption
coefficient of that frequency band.  This equation is simply a rearrangement of
+@eq:alpha. These per-band reflectances are converted to per-band
normal-incidence impedances using +@eq:xi_0.  Finally, the impedances are
converted back to *angle-dependent* reflectances by +@eq:r_normal_incidence.
This is the same approach taken in [@southern_room_2013]. The angle of
incidence must be found for each individual reflection, by taking the inverse
cosine of the dot product between the incident ray direction and the surface
normal, when both are unit vectors.

The contribution $g$ of a single image source with intermediate surfaces $m_1
m_2 \dots m_n$ is given by

$$g_{m_1 m_2 \dots m_n} = \frac{\sqrt{Z_0/4\pi}}{d_{m_1 m_2 \dots m_n}}
\cdot R_{m_1} \ast R_{m_2} \ast \dots \ast R_{m_n} \ast \delta(\frac{d_{m_1 m_2
\dots m_n}}{c})$$ {#eq:}

where $Z_0$ is the acoustic impedance of air, $c$ is the speed of sound,
$d_{m_1 m_2 \dots m_n}$ is the distance from the receiver to the image source,
and $R_{m_i}$ is the reflectance of surface $i$.  This assumes that the
original source emits a pressure impulse $\delta$ at the starting-time of the
simulation.  The contributions of all image sources must be summed together to
find the final impulse response.

To create a digital audio file representing an impulse response, the output
must be discretised at some sampling frequency $f_s$.  The individual image
source contributions must be added, at positions corresponding to their
propagation delays, into an output buffer at that sampling frequency.  The
ideal buffer position for a given contribution is equal to $\tau f_s$ where
$\tau$ is the propagation delay of that contribution, equal to the total ray
distance divided by the speed of sound.  However, this value is unlikely to be
an integer, and so may not coincide with a sample index.  The simplest solution
would be to round to the closest integer, and use this as the sample index.
However, for applications such as multi-microphone simulation which are
sensitive to arrival time, this can lead to phase errors.  A better solution is
suggested by Fu and Li [@fu_gpu-based_2016]: The contribution can be positioned
with sub-sample accuracy, by replacing the impulsive $\delta$ signal with the
impulse-response of an ideal low-pass filter, with cut-off equal to the output
Nyquist frequency.  Such an impulse response is infinitely long, but tends to
zero quickly, so a Hann window can be applied to limit its length.  This form
of the impulse is as follows:

$$
\delta_{\text{LPF}}(n - \epsilon)=
\begin{cases}
	\frac{1}{2}(1+\cos\frac{2\pi (n - \epsilon)}{N_w})\text{sinc}(n - \epsilon), & - \frac{N_w}{2} < n < \frac{N_w}{2} \\
	0, & \text{otherwise}
\end{cases} 
$$ {#eq:}

where $n$ is an index in the output buffer, $\epsilon$ is the centre of the
impulse in samples ($\epsilon=\tau f_s$), and $N_w$ is the width of the window
in samples.

Each image-source contribution has per-band pressure values.  Rather than
summing all contributions directly to the output buffer, several buffers are
created, one per frequency band.  The contributions for each band are summed
into each buffer individually (item 6 in the figure).  The final output of the
simulation is created by band-passing and then mixing down the buffers (item
7). A single method for multi-band filtering is used throughout Wayverb, and
more details are given in the [Ray Tracer]({{ site.baseurl }}{% link
ray_tracer.md %}) section.

## Summary

The image source model can be used to find the path lengths and pressures of
purely specular reflections. It cannot model diffuse reflections, and late
reflections are generally diffuse. Therefore, the image source model is only
suitable for predicting early reflections.  The naive implementation of the
image source model has exponential complexity, and a great deal of the
computations are redundant. For this reason, a ray-tracing-based implementation
with greatly improved complexity has been developed. This implementation is
more efficient than the naive implementation (i.e. it does less redundant work)
although it may fail to find some valid image sources if the number of rays is
too low. This has been deemed a reasonable trade-off. The implementation is
also designed to re-use ray paths found by the ray-tracer model, minimising
duplicated work between the two models.  Finally, implementation details such
as a method for frequency-dependent boundary modelling, and sub-sample-accurate
impulse placement, have been described.
