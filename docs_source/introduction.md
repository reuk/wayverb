---
layout: page
title: Introduction
navigation_weight: 0
---

---
suppress-bibliography: true
...

# Introduction {- .major}

The aim of impulse response synthesis is to simulate the reverberant properties
of a space without having to physically build anything.
This is useful for a variety of applications: architects need to be able to
evaluate the acoustics of a building before construction begins, sound editors
for film sometimes need to mix in recordings which were not made on location,
electronic musicians like to conjure imaginary or impossible spaces in their
music, and virtual-reality experiences must use audio cues to convince the user
that they have been transported to a new environment.

Unfortunately, software allowing accurate binaural impulse responses to be
synthesised is not currently widely available.
Often, software produced for research purposes is never made public.
Such software that *is* available generally suffers from one or more of an array
of issues.

Most software relies only on fast geometric methods, which are inaccurate,
especially at low frequencies.
Conversely, programs opting to use more accurate wave-modelling methods require
long time periods, on the order of days, or significant computing power to run.

Licensing is also an problem.
Most room-acoustics packages are the product of years of combined research by
multiple contributors, which is only made viable by releasing the software
commercially.
However, this inhibits further research, as the code is not freely available.
This model also limits users to those able to pay, restricting widespread
adoption.

When software is made available freely, often the user experience suffers.
Code requires manual compilation, or can only be run from a textual interface,
or the project is outdated and unmaintained.

The Wayverb project provides a solution to these problems, by making available a
graphical tool for impulse response synthesis.
It combines several simulation techniques, providing an adjustable balance
between speed and accuracy.
It is also free to download, can be run immediately on commodity hardware,
and the source code can be used and extended under the terms of the GNU GPL
license.

This thesis will begin by examining common methods of room simulation and the
software which implements these methods, explaining why particular techniques
were chosen for Wayverb.
Then, each of the chosen techniques will be explored in depth, along with a
description of their implementation.
The procedure for producing a single impulse response from the outputs of
multiple modelling techniques will be detailed.
Two extensions to the basic room acoustics model will be described,
namely frequency-dependent reflections at boundaries, and
microphone/head-related transfer function (HRTF) simulation.
The project will be evaluated, and finally, avenues for future development will
be examined.
