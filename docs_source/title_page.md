---
title: >
    Wayverb: A Graphical Tool for Hybrid Room Acoustics Simulation

subtitle: |
    A thesis submitted to the University of Huddersfield in partial fulfilment
    of the requirements for the degree of Master of Arts

institute: Creative Coding Lab at the University of Huddersfield

author: Matthew Reuben Thomas
date: January 2017

abstract: |

	Musicians and sound designers use reverberation effects to impart a sense of
	space and realism to their compositions. Convolution reverb tools can be used
	to produce these high-quality reverb effects. However, to evoke the
	characteristics of a particular room, the impulse response of that room must be
	recorded. This process is time consuming and requires specialist equipment,
	which may render the process impractical. Architects may often face a similar
	problem: they require some method of evaluating whether rooms will meet
	exacting acoustic specifications, before building commences. In this case, it
	is impossible to record the impulse response, because the room doesn't exist
	yet.

	Both problems can be solved by acoustic simulation, which predicts acoustic
	behaviour using virtual 3D models. These simulations can be carried out on
	commodity computing hardware, and are faster, cheaper, and more convenient than
	building and recording a physical space. Currently, users are limited in their
	choice of simulation software, as existing solutions focus either on speed or
	on overall correctness. Fast simulations are based on geometric methods, which
	are intrinsically inaccurate at low frequencies, while accurate techniques are
	based on wave-modelling, which becomes prohibitively slow at higher
	frequencies. There is a clear need for a program which is able to produce
	accurate results quickly, and which is easy to use without specialist training.

	This project identifies a hybrid acoustic simulation technique which combines
	the efficiency of geometric simulation with the accuracy of wave-based
	modelling. This fusion of simulation techniques, not available in any existing
	piece of simulation software, gives the user the flexibility to balance
	accuracy against efficiency as they require. This hybrid method is implemented
	in the Wayverb program. For increased performance, Wayverb uses graphics
	hardware to accelerate calculations via parallelisation.  The program is made
	free and open-source, with a simple graphical interface, differentiating it
	from hybrid simulators found in the literature which are all private and
	closed-source. In this way, Wayverb is uniquely accurate, efficient, and
	accessible.

	The Wayverb program demonstrates that the hybrid method is efficient enough to
	be viable in consumer software, but testing reveals that simulation results are
	not directly ready for production usage. The differing properties of the
	wave-based and geometric methods can result in different onset and decay times
	in the upper and lower regions of the output spectrum, which reduces the
	perceived quality of the output despite increased low-frequency accuracy.
	Avenues for further research are suggested in order to improve the quality and
	usability of the software.

language: en-GB
suppress-bibliography: false

toc: true

header-includes:
    - \usepackage[all, defaultlines=4]{nowidow}
...

<!--
header-includes:
    - \usepackage[all, defaultlines]{nowidow}

    - \usepackage{setspace}
    - \onehalfspacing
-->
