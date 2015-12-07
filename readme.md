Hybrid Waveguide + Raytracing IR Generator
==========================================

This is an ongoing research project, I'll have more to say here in a couple of
months.

Structure
=========

Important Folders
-----------------

* *lib* - waveguide library
* *rayverb* - raytracing library
* *common* - shared code for the waveguide and raytracing libraries
* *tests* - simple verification tests will live here soon
* *cmd* - the command-line tool itself

Other stuff
-----------

* *mic_test* - programs for replicating tests similar to those in "Simulation of
  Directional Microphones in Digital Waveguide Mesh-Based Models of Room
  Acoustics", Hacıhabiboglu, IEEE transactions on audio, speech, and language
  processing, vol. 18, no. 2, February 2010 (might be moved to *tests* soon)
* *bsp* - binary space partition to speed up the raytracer, might be rolled back
  into *rayverb* if I decide to finish it
* *python* - handful of programs to check/graph results. Will be removed if I
  find a good C++ graphing library
* *docs* - anything I feel like I should write about as I'm working

How it works
============

* provide a maximum estimation frequency
* calculate sampling frequency, which is directly related to valid frequency
  range
    * (factor of 1/4)
* calculate grid spacing, which is also related to sampling frequency
    * and the speed of sound in the modelling medium (air)
* build a mesh of nodes
    * find axis-aligned bounding-box for entire model
    * fill it with nodes with appropriate spacing
    * for each node, calculate whether it is inside or outside the model
        * requires models which are completely closed
    * optional - precompute neighbor nodes - probably faster as a one-off
      action rather than every step in the simulation
        * currently doing this on the CPU, could do it on the GPU
* designate an input and an output node
* for each time step
    * add the input signal to the current pressure value at the input node
    * for each neighbor node to the current node
        * add their current pressure
    * divide by two
    * subtract the previous pressure at the current node
    * multiply by attenuation factor
    * if this is the output node, write out the current pressure value
