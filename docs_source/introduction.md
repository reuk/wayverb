---
layout: page
title: Introduction
navigation_weight: 0
---

---
suppress-bibliography: true
toc: null
...

# Introduction {- .major}

The aim of room acoustics simulation is to simulate the reverberant properties
of a space without having to physically build anything.  This is useful for a
variety of applications: architects need to be able to evaluate the acoustics
of a building before construction begins; sound editors for film sometimes need
to mix in recordings which were not made on location; electronic musicians like
to conjure imaginary or impossible spaces in their music, and virtual-reality
experiences must use audio cues to convince the user that they have been
transported to a new environment.

Unfortunately, software allowing the synthesis of accurate impulse responses is
not currently widely available.  Often, software produced for research purposes
is not made public.  Such software that *is* available generally suffers from
one or more of an array of issues:

- Most software relies only on fast geometric methods, which are inaccurate,
  especially at low frequencies.  Conversely, programs opting to use more
  accurate wave-modelling methods require long time periods, in the order of
  days, or significant computing power to run.
- Licensing is also a problem.  Most room acoustics packages are the product
  of years of combined research by multiple contributors, which is only made
  viable by releasing the software commercially.  However, this inhibits 
  further research, as the code is not freely available.  This model also 
  limits users to those able to pay, restricting widespread adoption.
- When software is made available freely, often the user experience suffers.
  Code requires manual compilation, or perhaps can only be run from a textual
  interface, or else the project is outdated and unmaintained.

The Wayverb project provides a solution to these problems, by making available
a graphical tool for impulse response synthesis.  It combines geometric and
wave-modelling simulation techniques, providing an adjustable balance between
speed and accuracy.  It is also free to download, can be run immediately on
commodity hardware, and the source code can be used and extended under the
terms of the GNU General Public License (GPL).

The software has three main simulation engines, each with complementary
strengths and weaknesses. Each engine had to be thoroughly researched, and
implementation decisions evaluated, in order to balance the dual aims of
accuracy and efficiency. This process is described in detail for each engine
type, as is the process of automatically combining the engine outputs. Wayverb
also implements two extensions to the basic room acoustics model, namely
frequency-dependent reflections at boundaries, and microphone modelling, both
of which are reviewed in detail.
