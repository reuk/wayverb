---
layout: page
title: Image-source
navigation_weight: 2
---

---
reference-section-title: References
...

# Image-source {.major}

## Background

### Basic Method

The image-source method aims to find the purely specular reflection paths between a source and a receiver.
This relies on the simplifying assumption that sound propagates only along straight lines or "rays".
Sound energy travels at a fixed speed, corresponding to the speed of sound, along these rays.
The intensity of each "packet" of sound energy decreases with $1/r^2$, where $r$ is the distance along the ray that the packet has travelled [@vorlander_auralization:_2007, p. 58].

Rays are perfectly reflected at boundaries.
When a ray is reflected, it spawns a secondary source "behind" the boundary surface.
This source is located on a line perpendicular to the wall, at the same distance from it as the original source, as if the original source has been "mirrored" in the surface.
This is a first-order reflection.
A ray which is reflected from several boundaries is represented by a "higher-order" image-source, which has been mirrored in each of those boundaries in turn [@kuttruff_room_2009, p. 104].

![Image sources are found by reflecting the source position in a boundary.\label{fig:image_source_construction}](images/image_source_construction)

All sources, original and image, emit the same impulsive source signal at the same time.
The total impulse response (i.e. sound pressure against time) is found by summing the signals from each source, delayed and attenuated appropriately depending on the distance between that source and the receiver.
The frequency response of the signal from each image source will additionally be modified depending on the characteristics of the boundaries in which that source was reflected.

Each image source represents a perfect specular reflection, so there is no way for the image model to calculate scattered or diffuse responses.
Though this may sound troubling, it is not very problematic.
The conversion of specular into diffuse sound energy is unidirectional, so repeated reflections cause the ratio of scattered to specular energy to increase monotonically.
It is shown in [@kuttruff_room_2009, p.126] that though the earliest reflections may be largely specular, after a few reflections the large majority of sound energy becomes diffuse.
This suggests that the image model should be used only for very early reflections, and a secondary model used to compute late, diffuse reflections.

### Validity Checks

Having found the position of an image-source, by reflecting it in one or more surfaces, it must be checked to ensure it represents a specular path to the receiver.
This is known as an *audibility test* [@vorlander_auralization:_2007, p. 202].

Consider first a source $S$, a receiver $R$, and a single wall $A$.
The source is reflected in $A$, creating an image-source $S_A$.
A line is constructed from $R$ to $S_A$.
If this line intersects $A$, then $S_A$ represents a valid image source.
Otherwise, there is no possible specular reflection involving $S$, $R$ and $A$.

Now consider two walls, $A$ and $B$.
The image source $S_{AB}$ has been reflected in $A$ then $B$.
For the image-source to be valid:

* $R \rightarrow S_{AB}$ must intersect $B$ at some point $B_\text{intersection}$,
* $B_\text{intersection} \rightarrow S_A$ must intersect $A$ at $A_\text{intersection}$, *and*
* $A_\text{intersection} \rightarrow S$ must not intersect with any scene geometry.

The validation of a third-order image-source will require three intersection checks, a fourth-order image will require four checks, and so on.
This method of tracing backwards from the receiver to each of the image sources is known as *backtracking*.

![**Left:** The paths $S \rightarrow A \rightarrow R$ and $S \rightarrow A \rightarrow B \rightarrow R$ are both valid. **Right:** $S \rightarrow B \rightarrow A \rightarrow R$ is an invalid path because $R \rightarrow S_{BA}$ does not intersect $A$.\label{fig:backtracking}](images/backtracking)

### Acceleration

The naive method to find all the image sources for a scene is very expensive.
Consider that to find a single first-order image source, the original source must be mirrored in a surface, and then an intersection test must be conducted between that surface and the image-source-to-receiver ray.
To find all first-order image sources, this process must be carried out for all surfaces in the scene.
To find all second-order image sources, each of those first-order images must be tested against every surface.
This continues for higher-order images, so that the number of checks for image sources of a given order is equal to the number of surfaces raised to the power of that order.
The relationship between the image-source order and the computation time is therefore exponential, meaning that high orders are impossible to compute in a reasonable time.

The majority of higher-order image sources found with the naive algorithm will be invalid.
That is, they will fail the visibility/intersection test.
For example, [@kuttruff_room_2009, p. 323] shows that, for tenth-order image-sources in a shoebox-shaped room, there are around 1.46e7 different image sources, only 1560 of which are valid.
If the invalid image-sources can be discarded early, without requiring individual checking, then the amount of computation can be greatly reduced to a viable level.
As explained above, image sources above order four or five are rarely required, but even these can be very time-consuming to find with the naive method.
Optimisations are, therefore, a necessity for any but the simplest simulations.

To accelerate the image-source process, [@vorlander_auralization:_2007] suggests tracing a large number of rays in random directions from the source, and logging the unique paths of rays which eventually intersect with the receiver.
Each unique path found in this way is used to generate an image source sequence, which is then checked as normal.
This technique has the advantage that the majority of surface sequences are *not* checked, so the image-source process is fast.
However, if the preliminary ray-tracer is not run with enough rays, it is likely to miss some valid paths, especially in complex scenes.
Additionally, if the receiver volume is too great, then some invalid paths may still be detected.

The technique used by Wayverb is similar to that presented in [@vorlander_auralization:_2007], but makes a small change.
A large number of random rays are traced, as before, but at each reflection point, the receiver is checked to see whether it is visible.
If it is, then the surface sequence is checked for a valid image-source.
This technique has two main advantages: more paths are checked, so it is more likely to find all the valid image-sources; and
ray paths don't have to be specular, so ray-tracing can use techniques like *vector-based scattering*.
The disadvantage is that a greater number of validity checks are required, though this number is still many times smaller than would be required by a naive implementation.

## Implementation

Here the concrete implementation of the image-source method is presented, as it is used in Wayverb.
Details of the microphone-modelling process are discussed separately, in the [Microphone Modelling]({{ site.baseurl }}{% link microphone.md %}) section.
The following image \text{(\ref{fig:image_source_process})} gives an overview of the entire process.

![Creation of an impulse response using image sources.\label{fig:image_source_process}](images/image_source_process)

The simulation prerequisites are:

* source position
* receiver position
* speed of sound in air
* acoustic impedance of air
* a scene, made up of triangles, where each triangle has an associated material comprised of multiband absorption and scattering coefficients (note that curved surfaces are not supported, and must be approximated by small triangles)

First, an axis-aligned bounding box is computed for the scene, and split into uniformly sized cuboid *voxels*.
Each voxel holds a reference to any triangles in the scene which happen to intersect with that voxel.
The voxel mesh acts as an "acceleration structure", speeding up intersection tests between rays and triangles.
To check for an intersection between a ray and a number of triangles, the simplest method is to check the ray against each triangle individually, which is very time consuming.
The voxel mesh allows the number to checks to be greatly reduced, by checking only triangles that are within voxels that the ray intersects.
These voxels can be found very quickly, by "walking" the voxels along the ray, using an algorithm presented in [@amanatides_fast_1987].
For large scenes with many triangles, this method can lead to speed-ups of an order of magnitude or more.
Assume all ray-intersection tests mentioned throughout this thesis use the voxel-acceleration method, unless explicitly noted.

Rays are fired in uniform random directions from the source.
Each ray is checked for an intersection with the scene, and if an intersection is found, some data about the intersection is recorded.
Specifically, the record includes the triangle which was intersected, and whether or not the receiver is visible from the intersection point.
Then, the vector-based scattering method [@christensen_new_2005] is used to find the directions of new rays, which are fired from the intersection points.
The ray-tracing process continues up to a certain depth, which is artificially limited to ten reflections in Wayverb.
For most simulations, three or four reflections should be adequate, though this depends somewhat on the scattering coefficients of the surfaces, as explained in the [Basic Method] subsection.

The ray tracer produces a list of reflection paths for each ray.
Some rays may follow the same paths, and so duplicates must be removed.
This is achieved by condensing per-ray information into a tree of valid paths.
Each node in the tree stores a reference to a triangle in the scene, and whether or not the receiver is visible from this triangle.
Each unique path starting from a root node in the tree represents a possible image source contribution, which must be checked.
This checking is carried out using the backtracking method explained above.
A nice property of the tree structure is that it can be traversed using depth-first recursion, allowing the results of some intermediate calculations to be cached between ray paths, speeding up the calculation with only minimal memory overhead.
This is similar to the approach mentioned in [@savioja_overview_2015].
Also, because the tree is immutable, it can be shared between multiple worker threads, which independently check each branch for valid image sources.
The nature of the recursive algorithm makes it a poor fit for an OpenCL implementation, so native (CPU) threads are used instead.

Some paths in the tree may not actually produce valid image sources, and these paths are discarded.
For paths which *do* contribute valid image sources, the propagation delay and frequency-dependent pressure of the image source signal must be found.
According to [@kuttruff_room_2009, p. 325], the propagation delay is equal to the distance from the receiver to the image source, divided by the speed of sound.
The pressure content is found by convolving together the reflectances of all intermediate surfaces.
This is equivalent to a single multiplication per frequency band, as long as reflectances can be represented by real values.

The surface reflectances are found by converting per-band absorptions into per-band normal-incidence reflectance magnitudes by $|R|=\sqrt{1-\alpha}$.
These are converted to per-band impedances by 

(@) $$\xi=\frac{1+|R|}{1-|R|}$$

Finally, the impedances are converted back to *angle-dependent* reflectances by

(@) $$R(\theta)=\frac{\xi\cos\theta-1}{\xi\cos\theta+1}$$

where $\theta$ is the angle of incidence at the surface.
This is the same approach taken in [@southern_room_2013].

The contribution $g$ of a single image source with intermediate surfaces $m_1 m_2 \dots m_n$ is given by

(@) $$g_{m_1 m_2 \dots m_n} = \frac{\sqrt{Z_0/4\pi}}{d_{m_1 m_2 \dots m_n}} \cdot r_{m_1} \ast r_{m_2} \ast \dots \ast r_{m_n} \ast \delta(\frac{d_{m_1 m_2 \dots m_n}}{c})$$

where $Z_0$ is the acoustic impedance of air, $c$ is the speed of sound, $d_{m_1 m_2 \dots m_n}$ is the distance from the receiver to the image source, and $r_{m_i}$ is the reflectance of surface $i$.
This assumes that the original source emits a pressure impulse $\delta$ at the starting-time of the simulation.
The contributions of all image sources must be summed together to find the final impulse response.

To create a digital audio file representing an impulse response, the output must be discretised at some sampling frequency $f_s$.
The individual image source contributions must be added, at positions corresponding to their propagation delays, into an output buffer at that sampling frequency.
The ideal buffer position for a given contribution is equal to $\tau f_s$ where $\tau$ is the propagation delay of that contribution.
However, this value is unlikely to be an integer, and so will not coincide with a sample index.
The simplest solution would be to round to the closest integer, and use this as the sample index.
However, for applications such as multi-microphone simulation which are sensitive to arrival time, this can lead to obvious phase errors.
A better solution is suggested in [@fu_gpu-based_2016]:
The contribution can be positioned with sub-sample accuracy, by replacing the impulsive $\delta$ signal with the impulse-response of an ideal low-pass filter, with cut-off equal to the output Nyquist frequency.
Such an impulse response is infinitely long, but tends to zero quickly, so it can be Hanning windowed to reduce the number of additions required.
This form of the impulse is as follows:

(@) $$
\delta_{\text{LPF}}(n - \epsilon)=
\begin{cases}
	\frac{1}{2}(1+\cos\frac{2\pi (n - \epsilon)}{N_w})\text{sinc}(n - \epsilon), & - \frac{N_w}{2} < n < \frac{N_w}{2} \\
	0, & \text{otherwise}
\end{cases}
$$

where $n$ is an index in the output buffer, $\epsilon$ is the centre of the impulse in samples ($\epsilon=\tau f_s$), and $N_w$ is the width of the window in samples.

Recall that each image-source contribution has per-band pressure values.
Rather than summing all contributions directly to the output buffer, several buffers are created, one per frequency band.
The contributions for each band are summed into each buffer individually.
The final output of the simulation is created by band-passing and then mixing down the buffers.

## Integration with Ray Tracing Algorithm

The beginning of the image-source process relies on randomly ray tracing a certain number of reflections.
This ray tracing process is similar to that used for estimating late, diffuse reflections.
When the simulation is run, rays are actually traced to a much greater depth of maybe 100 reflections or more.
The first few reflections are routed to image-source processing, while the entire set of reflections is used for finding the reverb tail.

It is important to note that the stochastic ray tracing process will record both specular and diffuse reflections.
At the beginning of the impulse response, this will lead to a duplication of energy, as the energy from specular reflections will be recorded by both the image-source and ray-tracing processes.
To solve this problem, the stochastic ray tracer records specular and diffuse contributions separately.
Specular contributions from the ray tracer are only added to the output for reflections of higher order than the highest image-source order.

A second problem is surface scattering.
When simulating scenes with high surface scattering coefficients, specular reflections should be quiet, with a greater degree of scattered energy.
Unfortunately, the image-source process cannot account for scattered sound energy by design.
The solution is to use diffuse contributions from the stochastic ray tracer, so that the image-source and ray-traced outputs "overlap".
To ensure that the amount of energy in the simulation remains constant, the image-source finder must account for energy lost to scattering during reflections.
Therefore, after finding the reflectance of each surface using the method outlined above, the reflectance is further multiplied by $(1 - s)$ where s is the frequency-dependent scattering coefficient of the surface.
This causes the image-source contributions to die away faster, and the "missing" energy will be made up by the diffuse output of the ray tracer.

<!--

## Testing

TODO compare "exact" method to new method in shoebox model

-->
