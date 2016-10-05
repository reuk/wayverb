wayverb
=================================================================

*hybrid waveguide and raytracing room acoustics on the GPU*

Project Structure
=================

Important Folders
-----------------

* **lib**: all the library code for the project. This is further subdivided:
    * **common**: generic utilities such as data structures, architectural
      patterns and DSP helpers
    * **raytracer**: components which relate specifically to geometric acoustics
    * **waveguide**: components which relate specifically to FDTD air pressure
      simulation
    * **combined**: one way of combining the raytracer and waveguide components
      for broadband room acoustics simutions

* **utils**: a collection of programs primarily for testing outputs from the
  library components

* **wayverb**: a GUI interface to the `combined` library written with JUCE

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

Requirements
============

This project has been tested on macOS 10.11.6.

The library code *should* be platform-independent, but relies on experimental
language features from C++17, so you'll need a recent compiler to build it.
It also links against the OpenCL framework.
It should be possible to build on Linux by modifying the 'opencl' section of
`dependencies.txt` to find your system's opencl drivers.

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
mkdir -b build      # create a folder to hold built products
cd build            # move to that folder
cmake ..            # run cmake to configure the build
make                # run the build itself
```

The first time you run this, cmake will download all the project's dependencies
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
