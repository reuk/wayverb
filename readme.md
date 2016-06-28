wayverb
=================================================================

*hybrid waveguide and raytracing room acoustics on the GPU*

This is an ongoing research project, I'll have more to say here in a couple of
months.

Structure
=========

Important Folders
-----------------

* *waveguide* - waveguide library
* *rayverb* - raytracing library
* *common* - shared code for the waveguide and raytracing libraries
* *tests* - simple verification tests will live here soon
* *cmd* - the command-line tool itself
* *visualiser* - precursor to an eventual GUI tool for configuring simulations

Other stuff
-----------

* *mic_test* - programs for replicating tests similar to those in "Simulation of
  Directional Microphones in Digital Waveguide Mesh-Based Models of Room
  Acoustics", Hacıhabiboglu, IEEE transactions on audio, speech, and language
  processing, vol. 18, no. 2, February 2010 (might be moved to *tests* soon)
* *boundary_test* - program for testing frequency-dependent boundaries in the
  rectilinear waveguide mesh, and for graphing results.
* *python* - handful of programs to check/graph results. Will be removed if I
  find a good C++ graphing library
* *docs* - anything I feel like I should write about as I'm working

Updating the JUCE project
=========================

1. Make necessary changes to .jucer file, then click 'save + open'
2. Run `./mkxcproj.sh` to generate an xcode project for the wayverb engine
3. Drag `xcproj/WAYVERB.xcodeproj` to the left bar in xc project that the jucer
   opened previously.
4. In the wayverb xc project settings, go to `Build Phases` and click on
   `Link Binary With Libraries`.
5. Add all the libraries from the engine xc project.
