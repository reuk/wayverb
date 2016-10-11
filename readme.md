Wayverb
=======

*hybrid waveguide and ray-tracing room acoustics with GPU acceleration*

Synopsis
========

This project contains a library for offline room-acoustics simulations, along
with a graphical app which can be used to set-up and run these simulations.
The app produces room impulse responses, which can be used with convolution
reverbs to create realistic auralisations of virtual spaces.
Simulated room impulse responses may be particularly useful for architects,
sound-designers, and musicians.

There are several common methods for simulating room acoustics, which can
largely be subdivided into two main categories:

* **Geometric methods** are fast but inaccurate, especially at low frequencies.
* **Wave-modelling methods** are much more accurate, but time-consuming to
  compute, especially at high frequencies.

As the strengths and weaknesses of the two methods balance one-another out, it
makes sense to combine both methods, so that wave-modelling is used to simulate
low-frequency output, and geometric methods are used to generate high-frequency
content.

The approach of this library is to use:

* **image-source** for high-frequency early reflections,
* **stochastic ray-tracing** for high-frequency late reflections, and
* **rectilinear waveguide mesh** for all low-frequency content.

Requirements
============

This project has been tested on macOS 10.11.6.

The library code *should* be platform-independent, but relies on experimental
language features from C++17, so you'll need a recent compiler to build it.
It also links against the OpenCL framework.
It should be possible to build on Linux by modifying the 'OpenCL' section of
`dependencies.txt` to find your system's OpenCL drivers.

While this project *might* work on a mac with integrated graphics, ideally you
should use a recent mac with a discrete graphics card.
You could be waiting a long time otherwise!

Build Instructions
==================

Library Only
------------

This project has a fairly standard CMake build process.
You should be able to `git clone` a copy of the repository, then from the
project folder run:

```
mkdir -p build      # create a folder to hold built products
cd build            # move to that folder
cmake ..            # run cmake to configure the build
make                # run the build itself
```

The first time you run this, CMake will download all the project's dependencies
and build local copies of them.
The build will be quite slow for this reason (depending on the speed of your
internet connection).

Graphical App
-------------

The app is built with JUCE, and JUCE likes to own its own build.

An Xcode project is included, which you should be able to open and run in the
normal way.

If you have a copy of the Projucer installed, you're welcome to try generating
a Linux project from the included `wayverb.jucer`, but it's not guaranteed to
work.

Project Structure
=================

Important Folders
-----------------

* **library**: all the library code for the project. This is further subdivided:
    * **common**: generic utilities such as data structures, architectural
      patterns and DSP helpers
    * **raytracer**: components which relate specifically to geometric acoustics
    * **waveguide**: components which relate specifically to finite-difference
      time-domain (FDTD) air pressure simulation
    * **combined**: one way of combining the ray-tracer and waveguide components
      for broadband room acoustics simulations

* **wayverb**: a GUI app interface to the `combined` library written with JUCE

* **utils**: a collection of small command-line programs primarily for testing
  outputs from the library components

Other Folders
-------------

* **scripts**: a 'scratchpad' folder for python and octave prototypes

* **submodules**: This project has several dependencies. For the most part,
  these have official distributions, which CMake can fetch automatically when
  the project is configured. This folder holds supporting code which has no
  official distribution.

* **demo**: assets for testing purposes

* **config**: These files configure the Travis CI process which automatically
  builds and publishes the library documentation.

License
=======

Please see the `LICENSE` file for details.
