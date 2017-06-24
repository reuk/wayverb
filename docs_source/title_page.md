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

  # Abstract {- .major}

  Acoustic simulation aims to predict the behaviour of sound in a particular
  space.  These simulations can be carried out on commodity computing hardware,
  and are faster, cheaper, and more convenient than building and recording a
  physical space.  This is useful to architects, who need to know that their
  designs will meet exacting specifications before building starts. It is also
  useful to musicians and sound designers, who can use the simulation results to
  create pleasing, precise, and immersive audio effects.

  Currently, users are limited in their choice of simulation software, as
  existing solutions focus either on speed or on overall accuracy. Fast
  simulation techniques are often inaccurate at low frequencies, while more
  accurate techniques become prohibitively slow at higher frequencies. There is a
  clear need for a program which is fast, accurate at all frequencies, and easy
  to use without specialist training.

  This project identifies a hybrid acoustic simulation technique which combines
  the efficiency of geometric simulation with the accuracy of wave-based
  modelling. This fusion of simulation techniques, not available in any existing
  piece of simulation software, gives the user the flexibility to balance
  accuracy against efficiency as they require. This hybrid method is implemented
  in the Wayverb program. The program is made free and open-source, with a simple
  graphical interface, differentiating it from hybrid simulators found in the
  literature which are all private and closed-source. In this way, Wayverb
  is uniquely accurate, efficient, and accessible.

  Rather than presenting an entirely new simulation method, the Wayverb project
  surveys algorithms from the literature, employing those which are deemed most
  appropriate. It focuses on issues of practical implementation. In particular,
  for increased performance, Wayverb uses graphics hardware to accelerate
  calculations via parallelisation. Test data is presented to demonstrate
  accuracy.

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

    - \usepackage{setspace}
    - \onehalfspacing
...

<!--
header-includes:
    - \usepackage[all, defaultlines]{nowidow}
-->
