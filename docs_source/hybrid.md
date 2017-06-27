---
layout: page
title: Hybrid Model
navigation_weight: 5
---

---
reference-section-title: References
...

# Hybrid Model {.major}

## Background

The previous sections have looked at the theory and implementation of three
different acoustic simulation techniques.  The image-source method is accurate
for early reflections, but slow for longer responses.
The ray tracing method is inaccurate, but produces acceptable responses for
"diffuse" reverb tails.  The waveguide method models physical phenomena better
than the geometric methods, but is expensive at high frequencies.  By combining
all three models, accurate broadband impulse responses can be created, but for
a much lower computational cost than would be possible with any individual
method.

This section will focus on the two most important factors governing the
combination of modelling techniques. The first is positioning transitions: in
the time domain, from early to late reflections; and in the frequency domain,
between geometric and waveguide modelling. The second is matching the output
levels of the different methods, so that there are no sudden discontinuities in
level, and the transitions are seamless.

## Transitions

### Early and Late Reflections

The beginning of the image-source process relies on randomly ray tracing a
certain number of reflections.  This ray tracing process is similar to that
used for estimating late, diffuse reflections.  When the simulation is run in
Wayverb, rays are actually traced to a depth of 100 reflections or more.  The
first few reflections are routed to image-source processing, while the entire
set of reflections is used for finding the reverb tail.

It is important to note that the stochastic ray tracing process will record
both specular and diffuse reflections.  At the beginning of the impulse
response, this will lead to a duplication of energy, as the energy from
specular reflections will be recorded by both the image-source and ray-tracing
processes.  To solve this problem, the stochastic ray tracer records specular
and diffuse contributions separately.  Specular contributions from the ray
tracer are only added to the output for reflections of higher order than the
highest image-source order.

Surface scattering also poses a problem.  When simulating scenes with high
surface scattering coefficients, specular reflections should be quiet, with a
greater degree of scattered energy.  Unfortunately, the image-source process
cannot account for scattered sound energy by design.  The solution is to use
diffuse contributions from the stochastic ray tracer, so that the image-source
and ray-traced outputs "overlap".  To ensure that the amount of energy in the
simulation remains constant, the image-source finder must account for energy
lost to scattering during reflections.  Therefore, after finding the
reflectance of each surface using the method outlined above, the reflectance is
further multiplied by $(1 - s)$ where s is the frequency-dependent scattering
coefficient of the surface.  This causes the image-source contributions to die
away faster, and the "missing" energy will be made up by the diffuse output of
the ray tracer.

The transition between the image-source and ray tracing models will generally
overlap. The image-source response will fade away as the ray-traced diffuse
reflections become louder. The exact number of early reflections to be found
with the image-source method is largely a subjective decision.  For diffuse
rooms, the early specular reflections will be very quiet, regardless of which
method is used, so it is appropriate to set the number of specular reflections
very low, or even to disable image-source contributions altogether. For rooms
with surfaces which are large, smooth, and flat, specular reflections will form
a more significant part of the room response, and so it is reasonable to use
deeper image-source reflections in this case. Even under these conditions, an
image-source depth of more than 5 or 6 is unnecessary: in virtually all scenes,
some incident sound energy will be scattered diffusely, and the conversion of
"specular energy" into "scattered energy" is unidirectional, meaning that late
reflections in all scenes will be diffuse, and therefore suitable for
simulation with stochastic ray-tracing methods [@kuttruff_room_2009, p. 126].

### Crossover Position

In the interests of efficiency, the most accurate waveguide method is only used
at low frequencies, where it is relatively cheap. The rest of the audible
spectrum is modelled with geometric methods, which are most accurate at higher
frequencies.  However, there is no concrete rule about where to place the
crossover between "low" and "high" frequencies in this context.  It should be
clear that, when the time and computing power is available, the cutoff should
be placed as high as possible, so as to use accurate wave-based modelling for
the majority of the output.  However, in practice, it might be useful to have
an estimate for the frequencies where wave-modelling is strictly required, to
guide the placement of the cutoff frequency.  The *Schroeder frequency* is such
an estimate, and is based on the density of room resonances or "modes".  Below
the Schroeder frequency, room modes are well separated and can be individually
excited.  Above, the room modes overlap much more, resulting in a more "even"
and less distinctly resonant sound.  The Schroeder frequency is defined as
follows (see [@kuttruff_room_2009, p. 84] for a detailed derivation):

$$2000\sqrt{\frac{RT60}{V}}$$ {#eq:schroeder}

Here, $RT60$ is the time taken for the reverb tail to decay by 60dB, and $V$ is
the room volume in cubic metres.  Note that the Schroeder frequency is
inversely proportional to the square root of the room volume.  This implies
that in larger rooms, the shift from modal to non-modal behaviour is lower, and
therefore wave-based methods will be required to compute a smaller portion of
the spectrum.  In the [Waveguide]({{ site.baseurl }}{% link waveguide.md %})
section the complexity of a waveguide simulation was given as $O(V f_s^3)$ for
a space with volume $V$, at sampling frequency $f_s$.  Given that $f_s$ is
proportional to the waveguide cutoff frequency, which in turn may be set
proportional to $V^{-\frac{1}{2}}$ by +@eq:schroeder, the waveguide simulation
complexity becomes $O(V \cdot V^{-\frac{3}{2}})$ or $O(V^{-\frac{1}{2}})$. This
implies that, for a waveguide simulation capped at the Schroeder frequency, a
larger room will be cheaper to simulate than a smaller one.

The Schroeder frequency is only an estimate. The actual frequency dividing
"resonant" and "even" behaviours will vary depending on the surface area,
absorption coefficients, and shape of the modelled space. The optimum crossover
frequency should also be guided by the accuracy and time constraints imposed by
the user. For this reason, the Schroeder frequency is not used to guide the
placement of the crossover frequency in Wayverb. Instead, the user may select
the maximum frequency modelled by the waveguide, along with an oversampling
ratio.  In this way, the user can use the waveguide to model as wide a
bandwidth as their time constraints allow.  The literature suggests that this
is a valid approach: [@southern_room_2013] suggests that "under ideal
conditions the FDTD method would compute the entire RIR in all bands to ensure
physical accuracy", and [@southern_spatial_2011] notes that further research is
required in order to find a more objective method for placing the crossover.
Finally, the systems described in [@southern_hybrid_2013] and
[@murphy_hybrid_2008] allow the crossover frequency to be placed arbitrarily.

### Combining Outputs

Once the geometric and waveguide outputs have been produced, they must be
combined into a single signal. This combination process requires that the
geometric and waveguide outputs have the same sampling frequency. However, the
waveguide sampling frequency will almost certainly be lower than the final
output sampling frequency, so the waveguide results must be up-sampled.
Wayverb uses the Libsamplerate library for this purpose.  The sampling rate
conversion preserves the signal magnitude, but not its energy level. The
re-sampled waveguide output is therefore scaled by a factor of $f_{s\text{in}}
/ f_{s\text{out}}$ (where $f_{s\text{in}}$ is the waveguide sampling rate, and
$f_{s\text{out}}$ is the output sampling rate), so that the correct energy
level is maintained.

Once the waveguide sampling rate has been corrected, the waveguide and
geometric outputs are filtered and mixed.  The filtering is carried out using
the frequency-domain filtering method described in the [Ray Tracing]({{
site.baseurl }}{% link ray_tracer.md %}) section. The waveguide is low-passed,
and the geometric outputs are high-passed, using the same centre frequency and
crossover width in both cases.  The final output is produced by summing the
filtered responses.

## Level Matching

### Image-Source and Ray Tracer

The [Image Source]({{ site.baseurl }}{% link image_source.md %}) model operates
in terms of pressure. This means that the pressure contribution of each
individual image is inversely proportional to the distance between that image
source and the receiver.  In contrast, the [Ray Tracing]({{ site.baseurl }}{%
link ray_tracer.md %}) method operates in terms of acoustic intensity, and the
total intensity measured at the receiver depends only on the number of rays
which intersect with it. The distance travelled by each ray is not taken into
account.

A method for equalising the output of the two models is given in
[@schroder_physically_2011, p. 75].  The goal of the method is to ensure that
the energy of the direct contribution (the wave-front travelling directly from
source to receiver, with no intermediate reflections) is equal between the two
models.

First, equations for the intensity at the receiver must be created.  Given a
source and receiver separated by distance $r$, the intensity of the direct
image-source contribution is given by:

$$E_{\text{image source}}=\frac{E_{\text{source}}}{4\pi r^2}$$ {#eq:}

This is the standard equation for describing the power radiated from a point
source.

For the ray tracing method, the intensity of the direct contribution is a
function of the number of rays $N$, and the intensity of each ray $E_r$.  Only
rays intersecting the receiver will be registered, so ray intensity must be
normalised taking into account the proportion of rays which will intersect the
receiver. For a spherical receiver, and uniformly distributed rays, the
proportion of rays which intersect the receiver can be estimated as the ratio
between the area covered by the receiver, and the total area over which the
rays are distributed. If the receiver is at a distance $r$ from the source,
with an opening angle $\gamma$, then its area is that of a spherical cap (see
+@fig:detected_energy):

$$ A_{\text{intersection}} = 2\pi r^2(1-\cos\gamma) $$ {#eq:}

Then, the total direct energy registered by the ray tracer can be expressed:

$$
\begin{aligned}
E_{\text{ray tracer}} & = NE_r \left( \frac{A_{\text{intersection}}}{4\pi r^2} \right) \\
                      & = NE_r \left( \frac{2\pi r^2(1-\cos\gamma)}{4\pi r^2} \right) \\
                      & = NE_r \left( \frac{1-\cos\gamma}{2} \right)
\end{aligned}
$$ {#eq:}

![The proportion of uniformly-distributed rays intersecting the receiver
depends on the distance to the source, and opening angle formed by the
receiver. The acoustic intensity registered by the ray tracer is given by the
number of rays which intersect the receiver, and the energy carried by each
ray.](images/detected_energy){#fig:detected_energy}

The direct energy in both models should be equal, so the two equations can be
set equal to one another, giving an equation for the initial intensity of each
ray, in terms of the source intensity $E_{\text{source}}$, the opening angle of
the receiver $\gamma$, and the number of rays $N$.

$$
\begin{aligned}
E_{\text{ray tracer}} &= E_{\text{image source}} \\
NE_r \left( \frac{1-\cos\gamma}{2} \right) &= \frac{E_{\text{source}}}{4\pi r^2} \\
E_r &= \frac{E_{\text{source}}}{2 \pi r^2 N (1 - \cos\gamma)}
\end{aligned}
$$ {#eq:}

As long as the initial ray intensities are set according to this equation, both
methods will produce outputs with the correct relative levels.  The outputs
from the two methods are combined by simple signal addition, with no need for
additional level adjustment.

### Geometric and Waveguide

Setting the waveguide mesh level should follow a similar procedure. The
waveguide output level should be normalised so that the direct intensity
observed at the receiver is equal to that observed in the image-source model.
The normalisation is achieved through the use of a *calibration coefficient*
which can be used to scale the waveguide output, or alternatively to adjust the
magnitude of the input signal.

Several suggestions for finding the calibration coefficient are suggested in
the literature. One option, which is perhaps the simplest (short of manual
calibration), is found in [@southern_spatial_2011].  The average magnitude of a
certain frequency band is calculated for both the waveguide and geometric
output. The calibration coefficient is then equal to the ratio of these two
magnitudes.  This approach is flawed, in that the same frequency band must be
used for both signals. This frequency band will therefore be towards the lower
end of the geometric output, which is known to be inaccurate (this is the
entire reason for hybrid modelling), and at the top end of the waveguide output
(which may be inaccurate due to numerical dispersion).  It is impossible to
compute an accurate calibration coefficient from inaccurate data.

Another method, suggested in the same paper [@southern_spatial_2011], is to
find the intensity produced by the waveguide at a distance of 1m, and then to
normalise both models so that they produce the same intensity at this distance.
The image-source direct response is low-pass filtered, so that it only contains
energy in the same frequency band as the waveguide output. Then, the maximum
magnitude in the initial portion (up to the first reflection) of the output
signal is found, for the image-source and waveguide output. The calibration
parameter is found by taking the ratio of these maximum magnitudes.  This
differs from the first technique, in that the calibration coefficient is
derived from a single contribution in the time-domain, instead of from
frequency-domain magnitudes accumulated over the entire duration of the signal.

The second method is superior, in that it will produce an accurate calibration
coefficient.  Unfortunately, it requires analysis of the waveguide output
signal which, while not time-consuming, seems unnecessary.  A given mesh
topology and excitation signal should always require the same calibration
coefficient, assuming that the geometric source level remains constant. It
should be possible to calculate the calibration coefficient for a certain mesh
topology, and then to re-use this coefficient across simulations. This is the
approach taken in [@siltanen_finite-difference_2013] which provides Wayverb's
calibration method.

This general calibration coefficient is found by exciting a waveguide mesh with
an impulsive signal, and recording the pressure at a receiver node immediately
adjacent to the source node.  The simulation continues until the magnitude of
vibrations at the receiver have reduced below some threshold (perhaps falling
below the noise floor).  Now, the change in pressure at a distance $X$ is
known, where $X$ is the inter-nodal spacing of the waveguide mesh.  The
geometric pressure level at the same distance is given by

$$ p_g=\sqrt{\frac{PZ_0}{4\pi}} / X $$ {#eq:}

where $P$ is the source strength and $Z_0$ is the acoustic impedance of air.
The waveguide pressure level cannot be directly compared to the geometric
pressure level, because the upper portion of the waveguide output frequency
range is invalid. Instead, the DC levels are compared. The DC component of the
waveguide output can be found simply by accumulating the signal at the
receiver. Now, the calibration coefficient $\eta$ can be expressed like so:

$$\eta = \frac{p_\text{init}R}{p_\text{DC}X}$$ {#eq:}

where $p_\text{init}$ and $p_\text{DC}$ are the initial and DC pressure levels
respectively, $X$ is the inter-nodal spacing, and $R$ is the distance at which
the *geometric* source has intensity 1W/m².

Experimentally-obtained values of $\frac{p_\text{DC}}{p_\text{init}}$ are given
in [@siltanen_finite-difference_2013] for several different mesh topologies.
To produce normalised waveguide outputs, a calibration coefficient is
calculated, using the experimental result corresponding to a rectilinear mesh.
The waveguide excitation is then scaled by this calibration coefficient.

#### Testing

To validate the waveguide calibration procedure, a simple cuboid-shaped room is
simulated using the image-source and waveguide methods. The outputs are
compared in the frequency-domain, to ensure that the modal responses of the two
models match, in shape and in magnitude.

Although geometric methods are generally not capable of modelling low-frequency
modal behaviour, the image-source model in a geometric room is a special case.
For cuboid rooms with perfectly reflective surfaces, the image-source method is
exact [@kuttruff_room_2009], and it remains reasonably accurate for
slightly-absorbing surfaces. In cuboid rooms the image-source model can,
therefore, predict modal behaviour. Additionally, for this room shape, the
image source method can be dramatically accelerated, making it possible to
calculate extended impulse responses [@allen_image_1979]. This accelerated
method differs from Wayverb's image-source finder, in that it can calculate
long impulse responses for one specific room shape, whereas Wayverb's can
calculate short responses for arbitrary geometry.

If the accelerated method is implemented, it can be used to generate impulse
responses which are close to ideal (depending on the surface absorptions used).
These impulse responses can be compared to those produced by the waveguide, and
if the calibration coefficient has been chosen correctly, then their frequency
responses should match.

<!-- TODO is it worth talking about implementation issues for the accelerated image-source? -->

A room, measuring $5.56 \times 3.97 \times 2.81$ metres is simulated, using the
accelerated image-source and waveguide methods. The source is placed at (1, 1,
1), with a receiver at (2, 3, 1.5). Both methods are run at a sampling rate of
16kHz.  The simulation is carried out three times, with surface absorptions of
0.2, 0.1, and 0.05, and in all cases the simulation is run up until the
Sabine-estimated RT60, which is 0.52, 1.03 and 2.06 seconds respectively.  The
resulting frequency responses are shown in +@fig:calibration.

![Frequency responses analysis of image-source and waveguide outputs. The
initial waveguide level has been calibrated using the technique described
above. Room mode frequencies are shown in
grey.](images/calibration){#fig:calibration}

In the graphs, room modes are shown. One of the properties of the waveguide is
that it models wave effects which directly contribute to this low-frequency
modal behaviour. In the case of arbitrarily-shaped rooms, the image model is
not exact, and cannot be used to model low-frequency behaviour in this way,
while the waveguide will accurately model low-frequency behaviour in any
enclosed space. This is the main reason for using a wave-modelling technique at
all, instead of using geometric methods for the entire spectrum.

Below 30Hz, the responses show significant differences. Between 30 and 70Hz,
the responses match closely, to within a decibel. There is some divergence at
80Hz, after which the results match closely again until the upper limit of
200Hz. At the upper end of the spectrum, the levels match closely between
outputs, but the peaks and troughs are slightly "shifted". 

The low-frequency differences can be explained as error introduced by the
hard-source/Dirac-delta waveguide excitation method (see [Digital Waveguide
Mesh]({{ site.baseurl }}{% link waveguide.md %})). This source type has
previously been demonstrated to cause significant error at very low frequencies
[@sheaffer_physical_2014].

There are several possible causes for the remaining differences seen between
the outputs of the different models. Firstly, the image-source technique is
only exact for perfectly reflecting boundaries. The boundary model used in this
image-source implementation is the same locally-reacting surface model that
Wayverb uses: the reflection factor is real-valued and angle-dependent. A more
physically correct method would be to use complex reflection factors, which
would allow phase changes at boundaries to be represented. The boundary model
is almost certainly the cause of the largest discrepancy, at around 80Hz:
results given in [@aretz_combined_2009, p. 78] show similar artefacts of the
real-value angle-dependent reflection factor, compared against other more
accurate image-source boundary types. Due to time constraints, these more
complicated boundary models could not be tested here.

The small frequency shift at the top of the spectrum is most likely to be
caused by numerical dispersion in the waveguide mesh. Numerical dispersion
becomes more pronounced as frequency increases, which is consistent with the
shift seen in the results, which is greater at higher frequencies.  The shift
may also be caused by slight differences in the dimensions of the modelled
room, or the source and receiver positions.  In the image-source model, all
measurements and positions are exact, but the waveguide must "quantise" the
boundary, source, and receiver positions so that they coincide with mesh nodes.
If the dimensions of the room were adjusted slightly, this would also cause the
room modes to change (which again would be more pronounced at higher
frequencies), which might lead to a perceived spectral shift relative to a room
with exact dimensions.

Despite the small differences between the frequency responses, the close level
match between models suggests that the calibration coefficient is correct.

The reverb times of the outputs are also compared and shown in
+@tbl:reverb_times.

Table: Reverb times of outputs generated by image source and waveguide models {#tbl:reverb_times}

-------------------------------------------------------------------------------
absorption          method              T20 / s             T30 / s
------------------- ------------------- ------------------- ------------------- 
0.05                exact image source  1.044               1.065

0.05                waveguide           1.165               1.180

0.10                exact image source  0.5401              0.5633

0.10                waveguide           0.5689              0.5905

0.20                exact image source  0.2768              0.2990

0.20                waveguide           0.2674              0.2880
-------------------------------------------------------------------------------

There is a difference of 11% for the lowest absorption, which falls to 6% for
an absorption of 0.10, and to 4% for an absorption of 0.20. Given that the
image-source method is nearly exact in cuboid rooms, these differences are
small enough to suggest that the waveguide and its boundary model have been
implemented correctly.
