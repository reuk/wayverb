---
layout: page
title: Evaluation
navigation_weight: 24 
---

---
suppress-bibliography: true
toc: null
...

# Conclusion {.major}

The goal of the Wayverb project was to create a program which was capable of
simulating the acoustics of arbitrary enclosed spaces. For the program to be
useful to its target audience of musicians and sound designers, it must be
simultaneously accurate, efficient, and accessible.

The aims of accuracy and efficiency would be met by combining wave-modelling
and geometric simulation methods, benefiting from both the physical realism of
wave-modelling, and the computational performance of geometric simulation.
This technique is not used by any other publicly available simulation package,
so it was thought that a program implementing both models would be both faster
and more accurate than competing programs. To further improve performance, the
simulation would be implemented to run in parallel on graphics hardware. The
program would be free and open-source, with a graphical interface, to ensure
accessibility and encourage adoption.

Testing shows that the individual modelling methods are individually reasonably
accurate. The ray-tracing and image-source methods respond appropriately to
changes in room size, material, source/receiver spacing and receiver type.
This is also true of the waveguide, which additionally is capable of modelling
low-frequency modal responses, taking wave-effects such as diffraction into
account. However, the accuracy of the waveguide is a drawback in some respects.
When waveguide outputs are combined with geometric outputs, the relative
inaccuracies of the geometric results are often highlighted by obvious
discontinuities in the blended spectrum.  Although use of the waveguide
increases accuracy at low frequencies, generated outputs may be less useful
than if generated entirely with geometric techniques, simply because of these
discontinuities. This indicates that the goals of the project were misguided:
an additional, primary goal of "usefulness" or "fitness" should have been
considered. Future work may seek to improve the match between the outputs of
the different models, perhaps sacrificing some low-frequency accuracy in the
interests of sound quality.

In terms of efficiency, simulations generally complete within minutes, rather
than hours or days, meeting the project's efficiency target. It is also
possible to observe the progression of the simulation, and to retry it with
less intensive parameters, if it is progressing too slowly.  Unfortunately, the
time taken to generate outputs is not necessarily reflected in the quality of
the results.  For example, it is disappointing to wait for ten minutes for an
impulse response, only to find that the output has markedly different reverb
times at the top and bottom of the spectrum. Good user experience relies on
users being able to generate results with acceptable quality as quickly as
possible. If the user has to tweak and re-render, waiting for several minutes
each time, before eventually finding appropriate settings, this translates to a
poor user experience. This may be solved in two ways: by improving the quality
of the outputs; and/or by further optimisation of the simulation algorithms.

The application has an accessible graphical interface. Although some desirable
features (such as built-in convolution and 3D editing) are missing, the
interface is focused and functional. It is possible to install and use without
specialist training.  Additionally, all code is open-source, allowing
collaboration and contribution from interested third-parties. While the
accuracy and efficiency goals were not conclusively met, it is clear that the
finished project is sufficiently accessible.

Most importantly, the Wayverb project demonstrates that the hybrid modelling
approach is viable in consumer software.
