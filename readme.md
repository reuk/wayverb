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

    mkdir -p build      # create a folder to hold built products
    cd build            # move to that folder
    cmake ..            # run cmake to configure the build
    make                # run the build itself

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

* **src**: all the library code for the project. This is further subdivided:
    * **core**: generic utilities such as data structures, architectural
      patterns and DSP helpers
    * **raytracer**: components which relate specifically to geometric acoustics
    * **waveguide**: components which relate specifically to finite-difference
      time-domain (FDTD) air pressure simulation
    * **combined**: one way of combining the ray-tracer and waveguide components
      for broadband room acoustics simulations
    * **audio_file**: wrapper round libsndfile. If I ever switch the soundfile
      library from libsndfile (to something with a more flexible license) then
      this module is all that will have to change.
    * **frequency_domain**: wrapper around fftw. If I ever switch to some other
      library, this is the only code that will have to change. There are also
      a few utilities to do with analysis and filtering here.
    * **hrtf**: small utility for generating hrtf data files from audio inputs.
    * **utilities**: small self-contained utilities which aren't really tied to
      this project, but they have to live somewhere.

* **wayverb**: a GUI app interface to the `combined` library written with JUCE

* **bin**: a collection of small command-line programs primarily for testing
  outputs from the library components

Other Folders
-------------

* **scripts**: a 'scratchpad' folder for python and octave prototypes

* **demo**: assets for testing purposes

* **config**: These files configure the documentation generator. They used to
  configure the Travis CI process which automatically built and published the
  library documentation.

License
=======

Please see the `LICENSE` file for details.

**Software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability, fitness
for a particular purpose and noninfringement.
In no event shall the authors or copyright holders be liable for any claim,
damages or other libility, whether in an action of contract, tort or otherwise,
arising from, out of or in connection with the software or the use or other
dealings in the software.**
