% Directional Receiver Modelling

Introduction
============

In the literature, two methods for modelling directional receivers in the
digital waveguide mesh (DWM) are presented.
This essay will compare and evaluate the two methods, and explain the choice of
method for a tetrahedral waveguide auralisation model, and describe the
implementation of this chosen method.

The Blumlein Difference Technique
=================================

The first method, described by @southern2007 and @southern2007_2, is designed
with the goal of encoding the 3D sound-field by means of spherical harmonic
decomposition to the second order, for use in ambisonic reproduction.

This method is based on sound intensity probe theory, which allows directional
velocity components of a sound field to be captured using two closely-spaced
omnidirectional receivers.
In the DWM, each node has an associated pressure, so two closely-spaced nodes
can be used to emulate a pair of omnidirectional receivers, and in turn used
to generate directional velocity components.
A full first-order B-format recording can be made using seven such 'pressure
sensors', arranged in a cross shape, with a single node at the centre, and
six nodes placed around it.
A second-order recording is achieved by adding further pressure-sensing nodes
in between the first-order nodes.

While this technique requires the modelling of multiple coincident directional
microphone capsules (as would be used to produce a 'real' ambisonic recording),
the only important orientations are the *relative* orientations of the capsules.
The position and orientation of the microphone array is not important, as a
post-processing decoding step will be used to extract the desired directional
information.
The technique put forward suggests aligning the pressure sensors with the
axial directions of the mesh, resulting in directional components which are
also aligned to the mesh.
For this reason, the technique is not very flexible when directly generating
directional components.

Additionally, this technique has some difficulty adapting to different mesh
topologies.
In a rectilinear mesh, the seven first-order nodes can be placed exactly, as
their positions will coincide with nodes in the mesh.
However, the second-order nodes will not fall exactly on existing nodes, and
instead will need to be 'snapped' to the closest mesh nodes.
This problem becomes worse in other mesh topologies, such as the tetrahedral
mesh, where it may be that very few of the sensor nodes can be exactly placed in
the desired positions.

It is suggested to increase the spatial sampling frequency of the mesh in order
to facilitate more exact placement of sensor nodes (by reducing the grid
spacing, the quantisation error in the sensor placement is also reduced).
The example given by @southern2007_2 uses a sensor spacing of 0.04 metres,
giving a maximum valid sampling frequency of 4.25kHz.
However, to allow accurate placement of these sensors, a grid with a spacing
of 0.0027m is used.
Such a grid would normally support a maximum valid sampling frequency of
176.4kHz for a single-sensor omnidirectional receiver.
This mesh oversampling is very costly, as the computational cost of the 3D DWM
increases cubically as the spacing of the mesh is reduced.

In conclusion, although promising for producing ambisonic recordings in
rectilinear meshes where computational time is not a great concern, this method
does not seem well-suited to producing fast, arbitrary directional recordings in
the tetrahedral mesh.

The Intensity Calculation Technique
===================================

* 
