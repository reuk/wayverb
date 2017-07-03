---
layout: page
title: Theory
navigation_weight: 1
---

---
reference-section-title: References
...

<!--

# Notes

- Room acoustic theory
- Metrics for success
- Consideration of creative contexts

-[x] p27 scattering coefficient
-[x] boundary modelling background stuff (remove from image source section)
-[ ] T20, T30, RT60, EDT
-[ ] sabine reverb time

-->

# Theory {.major}

This chapter explains some aspects of room acoustics theory, which will help to
clarify the implementation decisions governing the simulation techniques
discussed in later chapters.

## Waves and Media

Sound waves can be described completely by specifying the instantaneous
velocity of particle in the propagation medium. Not all particles will have the
same velocity, which causes fluctuations in density, pressure, and temperature,
which are dependent upon time, and position. *Sound pressure*, $p$, is the
difference between the "at rest" pressure $p_0$, and the pressure measured in
the medium at a particular position and time $p_\text{tot}$
[@vorlander_auralization:_2007 p. 9]:

$$p = p_\text{tot} - p_0$$ {#eq:sound_pressure}

Sound pressure is measured in pascals (1 Pa = 1 N/m²)
[@vorlander_auralization:_2007 p. 16].  The goal of room acoustic simulation is
to predict the change in sound pressure over time at a particular point in a
sound field.  A similar equation to +@eq:sound_pressure can be written for
change in density due to sound $\rho$, where $\rho_0$ is the static density of
the medium, and $\rho_\text{tot}$ is the instantaneous density:

$$\rho = \rho_\text{tot} - \rho$$ {#eq:}

For the purposes of room acoustics, it may be assumed that the propagation
medium is air. Sound propagation through liquids and sold structures will be
ignored.  The speed of sound in air, $c$, is approximately

$$c=(331.4 + 0.6\theta) \text{m/s}$$ {#eq:}

where $\theta$ is the temperature in degrees centigrade [@kuttruff_room_2009 p.
7]. In most real-world acoustics problems, variations in temperature will be
very small, and can be ignored. That is, the propagation medium (which is
generally air) is assumed to be homogeneous.  A propagation medium can be
specified by its *characteristic impedance* or *wave impedance* $Z_0$
[@vorlander_auralization:_2007 p. 14]:

$$Z_0 = \rho_0 c$$ {#eq:}

This quantity denotes the medium's resistance to pressure excitation, or
alternatively the pressure required to induce movement in the medium's
particles. For air, the characteristic impedance is generally around 400 kg/m²s
(although this depends on the speed of sound, and therefore the air
temperature) [@vorlander_auralization:_2007 p. 15].

The difference in sound pressure between the quietest audible sound and the
threshold of pain is around 13 orders of magnitude. To make the values involved
more manageable, sound pressure is usually given in terms of the *sound
pressure level* (SPL), which is measured on a logarithmic scale in decibels
(dB) [@kuttruff_room_2009 p. 23]:

$$\text{SPL} = 20\log_{10}\frac{p_\text{rms}}{p_0} \text{dB}$$ {#eq:}

Here, $p_\text{rms}$ is the *root mean square* sound pressure, and $p_0$ is
a reference pressure of $2 \times 10^{-5}$ Pa.

Wave propagation in a medium causes energy to be transported through that
medium.  This energy flow can be measured in terms of the energy transported
per second through an area of 1m² (W/m²), and is called *sound intensity*, $I$
[vorlander_auralization:_2007 p. 19]:

$$\vec{I} = \overline{p \vec{v}}$$ {#eq:}

Note that the overbar notation signifies time-averaging. The sound intensity
can also be given in terms of the *intensity level*, $L_I$
[@vorlander_auralization:_2007 p. 20]:

$$L_I = 10\log_{10}\frac{|\vec{I}|}{I_0}$$ {#eq:}

where $I_0 = 10^{-12} W/m^2$ is a reference intensity chosen to match the
levels of sound pressure and intensity in a plane wave.

For a harmonic wave, the temporal and spatial periods of the wave are related
by the speed of sound, $c$:

$$c = \lambda f = \frac{\lambda}{T}$$ {#eq:}

where $f$ is the frequency of the wave in Hz, $T$ is the temporal period, and
$\lambda$ is the spatial period or *wavelength*.

In the simulation presented in this project, sound waves are assumed to
propagate equally in all directions from a point-like source, or *monopole*.
The pressure wave, propagating with increasing radius, is known as a *spherical
wave*.

The pressure $p$ observed at distance $r$ and time $t$ from a monopole source
with signal strength $Q(t)$ is given by [@vorlander_auralization:_2007 p. 24]:

$$p(r, t) = \frac{\rho_0}{4\pi r}\dot{Q}\left(t-\frac{r}{c}\right)$$ {#eq:}

For harmonic waves, the total radiated sound power $P$ of a monopole source
is related to the sound intensity by [@kuttruff_room_2009 p. 15]:

$$P = 4 \pi r^2 I$$ {#eq:}

## Boundary Characteristics

Room acoustics is not just concerned with the behaviour of pressure waves in
air. Additionally, room acoustics problems normally bound the air volume with a
set of surfaces (walls, floors, baffles etc.), and a pressure wave incident
upon such a surface may be reflected and/or absorbed. The reflected pressure
waves generally lead to complex sound fields, which in turn form the particular
sonic "character" or *acoustic* of an enclosure.

Several assumptions are made to simplify the equations governing wave behaviour
at a boundary.  First, incident waves are assumed to be plane waves. This is
never true in the Wayverb app, as simulations use point sources which produce
spherical waves.  However, the curvature of the wave front may be ignored so
long as the source is not close to the reflecting surface [@kuttruff_room_2009
pp. 35-36] ("close" here will depend on the error constraints of the particular
simulation). Secondly, boundary surfaces are assumed to be flat, and infinitely
large [@kuttruff_room_2009 p. 35]. This is a valid approximation only if the
size of each surface is large relative to the longest wavelength in the
simulation.

### Magnitude and Phase

The reflection factor $R$ of a boundary is a complex value given by 

$$R=|R|\exp(i\chi)$$ {#eq:}

which describes a modification to the amplitude and phase of a wave reflected
in a boundary ($|R|$ is the magnitude term, $\chi$ is phase).

This factor depends both on the frequency and direction of the incident wave.
When $\chi = \pi$, $R$ is negative, corresponding to a phase reversal.  This
is known as a "soft" wall, but is rarely seen in room acoustics.  It is
reasonable to assume that reflections are in-phase in the majority of
architectural acoustics problems [@kuttruff_room_2009, p. 36].

The wall impedance $Z$ is defined as the ratio of sound pressure to the normal
component of particle velocity at the wall surface.  It is related to the
reflection factor by

$$R=\frac{Z\cos\theta-Z_0}{Z\cos\theta+Z_0}$$ {#eq:r_in_terms_of_z}

where $\theta$ is the angle of incidence, and $Z_0$ is the characteristic
impedance of the propagation medium, normally air.  In the case that the wall
impedance is independent of the wave angle-of-incidence, the surface is known
as *locally reacting*.  A locally reacting surface does not transmit waves
tangentially along the wall surface. In Wayverb, all surfaces are modelled as
locally reacting.

The absorption coefficient $\alpha$ of a wall describes the proportion of
incident energy which is lost during reflection. It is defined as

 $$\alpha =1-|R|^2$$ {#eq:alpha}

The *specific acoustic impedance* $\xi$ for a given surface is defined as the
impedance of that surface $Z$ divided by the acoustic impedance of the
propagation medium (air) $Z_0$.

$$\xi=\frac{Z}{Z_0}$$ {#eq:}

Inserting this equation into +@eq:r_in_terms_of_z gives:

$$R=\frac{\xi\cos\theta-1}{\xi\cos\theta+1}$$ {#eq:r_in_terms_of_xi}

where $\theta$ is the angle of incidence [@southern_room_2013].

For a general surface, $\xi$ will be a function of the incident angle.
However, in the case of a locally reacting surface, the impedance is
independent of the angle of incidence. The $\xi$ term in +@eq:r_in_terms_of_xi
can then be replaced by $\xi_0$ which represents the normal-incidence specific
impedance (which will be constant for all angles). Thus, the reflection factor
of a locally reacting surface is

$$R=\frac{\xi_0\cos\theta-1}{\xi_0\cos\theta+1}$$ {#eq:r_normal_incidence}

Surfaces in Wayverb are defined in terms of absorption coefficients. To express
the reflectance in terms of absorption, an equation for $\xi_0$ in terms of
absorption must be found, and substituted into +@eq:r_normal_incidence.

Assuming that the absorption coefficients denote normal-incidence absorption,
then by rearranging +@eq:alpha, the normal-incidence reflection coefficient
$R_0$ is given by

$$R_0=\sqrt{1-\alpha}$$ {#eq:r_mag}

$R_0$ can also be expressed by setting $\theta$ to 0 in
+@eq:r_normal_incidence:

$$R_0=\frac{\xi_0 -1}{\xi_0 +1}$$ {#eq:r_0}

To express $\xi_0$ in terms of $\alpha$, +@eq:r_0 is rearranged in terms of the
normal-incidence reflection coefficient:

$$\xi_0=\frac{1+R_0}{1-R_0}$$ {#eq:xi_0}

Then, +@eq:r_mag may be substituted into +@eq:xi_0 to give $\xi_0$ in terms of
$\alpha$.  This new definition of $\xi_0$ can then be used in conjunction with
+@eq:r_normal_incidence to define the angle-dependent reflection factor of a
locally reacting surface.

### Scattering

The reflection factor, absorption coefficient, and wall impedance describe the
behaviour of perfectly-reflected (specular) waves.  If the reflecting surface
has imperfections or details of the same order as the wavelength, as many
surfaces in the real world do, then some components of the reflected wave will
be *scattered* instead of specularly reflected.

Describing the nature of the scattered sound is more complicated than specular
reflections.  A common method is to use a *scattering coefficient*, which
describes the proportion of outgoing energy which is scattered, and which may
be dependent on frequency (see +@fig:scattering).  The total outgoing energy
$E_\text{total}$ is related to the incoming energy $E_\text{incident}$ by

$$E_{\text{total}}=E_{\text{incident}}(1-\alpha)$$ {#eq:}

Then the scattering coefficient $s$ defines the proportion of this outgoing
energy which is reflected specularly $E_\text{specular}$ or scattered
$E_\text{scattered}$:

$$
\begin{aligned}
E_{\text{specular}} & =E_{\text{incident}}(1-\alpha)(1-s) \\
E_{\text{scattered}} & =E_{\text{incident}}(1-\alpha)s
\end{aligned}
$$ {#eq:}

![Reflected components from a rough
surface.](images/scattering){#fig:scattering}

Alone, the scattering coefficient fails to describe the directional
distribution of scattered energy.  In the case of an ideally-diffusing surface,
the scattered energy is distributed according to Lambert's cosine law.  That
is, the intensity depends only on the cosine of the outgoing scattering angle,
and is independent of the angle of incidence (see +@fig:lambert).  More complex
scattering distributions, which also depend on the outgoing direction, are
possible [@christensen_new_2005; @durany_analytical_2015], but there is no
single definitive model to describe physically-accurate scattering.

![Lambert scattering. Scattered intensity is independent of incident
angle.](images/lambert){#fig:lambert}

## Impulse Response Metrics

## Summary

TODO
