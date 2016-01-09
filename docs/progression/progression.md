% Progression Document
% Virtual Acoustics - Reuben Thomas

<!--

* annotated bibliography with key texts I've looked at
    * perhaps include future reading

* details of work undertaken so far (word counts, projects etc.)
    * with a plan for the Spring term

* present my primary focus, working methods

-->

Work Undertaken
===============

The Story So Far
----------------

The goal of the project is to write a program which can produce accurate,
musical impulse responses of virtual environments, without requiring excessive
computation time.

The proposed solution is to write a digital waveguide mesh simulation, which can
be used to model the lower range of the spectrum (up to 500-600Hz, for example).
This can then be combined with the acoustic ray tracer that I wrote for my
undergraduate individual project.
Finally, a graphical interface can be written which allows musicians to interact
with the software, rather than restricting users to a clunky command-line
interface.

At this time I have written a library which allows tetrahedral waveguide meshes
to be constructed and executed on the GPU.
This means that at each 'step' of the waveguide calculation, many nodes can
be updated in parallel, instead of having to update them sequentially on the
CPU.

The main research areas that I identified with the waveguide (beyond the
parallel implementation) were directional receiver modelling and boundary
condition modelling.
Of these, I have implemented directional receiver modelling, using the technique
put forward by @hacihabiboglu2010.
Additionally, I have written a short report in which I test the model and
compare my findings against those presented in the paper.
For the most part, the technique appears to work, but it has some issues at the
extreme ends of the spectrum.
In any case, the chosen technique was the most suitable out of the techniques
presented in the literature, and this implementation will be sufficient.

I was going to move directly on to boundary condition modelling, but was
put off by the density of the papers on the subject.
Therefore, I refocussed my efforts, for the time being, on writing a graphical
viewer/debugger for the waveguide and ray tracing components.
Though not directly a 'research' topic, this debugger will allow me to work
with greater ease on the identified research areas.
Already, it has helped me to find a few bugs in the waveguide which would have
gone unnoticed otherwise.
Finally, this program can serve as the starting-point for the graphical editor
which was one of my goals.

As a byproduct of writing the debugger, it became apparent that there were some
shortcomings in the efficiency of the ray tracer which would require fixing
before the project was complete.
I have therefore spent some time factoring out common code in the waveguide
and ray tracer code-bases, and modifying the ray tracer to remove some of the
implementation/performance bottlenecks.
Combining the two code-bases also allowed me to produce some preliminary hybrid
impulse responses, by combining the outputs of the two simulations by ear.
The work completed so far will form a solid starting point for a later goal of
the project, namely producing hybrid responses without needing to mix by ear.

Planned Work for the Next Term
------------------------------

Once the ray tracer refactoring is complete (I aim to have this done in the next
week or so), I will resume work on the modelling of boundary conditions in the
waveguide mesh.
I expect this to take a month or two.
I will produce a report, similar to the report on microphone modelling, on my
final implementation.

After this, I will begin working on combining the two models, which will
probably be a few weeks of work.
I will produce a report on this work too.

Finally, I will extend the graphical debugger into a graphical editor for
setting up and running complete simulations.

Annotated Bibliography
======================

### @duyne

Though not the first paper to discuss 3D digital waveguide meshes (DWM),
this is one of the first texts that focuses on the *tetrahedral* mesh.
Though less straightforward to implement than the rectilinear DWM, the
tetrahedral mesh is arguably better suited for high-performance, low-error
applications, as tested by @campos2005 (see below).

@duyne put forward a finite difference scheme for the tetrahedral DWM, which
is reasonably simple to implement in a digital signal processing (DSP) context.
They also prove that this scheme approximates the 3D lossless wave equation,
which is not particularly interesting from an implementer's viewpoint, but does
at least show that the DWM model is suitable for simulating wave effects
such as interference and diffraction.

Finally, a method for calculating the dispersion error of the mesh is given,
which is useful in cases where a given maximum dispersion error is required.
In such cases, the spatial sampling frequency of the mesh may be reduced until
the dispersion error at the top of the output bandwidth is within acceptable
limits.

### @campos2005

This paper compares the computational efficiency of several different mesh
topologies - rectilinear, tetrahedral, cubic close-packed (CCP), and octahdral.
In particular, the dispersion and density characteristics of each topology are
measured and compared.

It is shown that the rectilinear mesh has the highest sampling frequency, while
the tetrahedral mesh has the lowest.
However, the tetrahedral mesh's dispersion properties, alongside its relatively
low number of interconnections (and hence calculations) per node mean that for
applications where low dispersion error is required (below 22.8%), the
tetrahedral topology is the most efficient.

It should be noted that the memory requirements discussed in this paper do not
hold true for all mesh implementations.
The paper discusses DWMs based on *W-models*, in which each node stores an input
and an output variable for each immediately adjacent node.
In such meshes, the memory usage obviously scales with the number of
interconnections per node, or *coordination number* (so will be low for
tetrahedral meshes, which has a coordination number of 4, but will be much
higher for CCP meshes, which has a coordination number of 12).
However, a linear transformation can be applied to the W-DWM, converting it into
an alternate *K-variable* DWM, or K-DWM.
This formulation is equivalent to the W-DWM, but requires only a pair of
variables to be stored per node, meaning that memory requirements of two K-DWM
meshes of equal size and density but different coordination number will be
equivalent.
For more information on W- and K-DWMs, see @beeson2007.

### @hacihabiboglu2010

This paper presents a fast and simple method for modelling direction- and
frequency-dependent receivers in the DWM.
Such a method might be used to model simple polar-pattern receivers, specific
microphones, or even approximate HRTFs.

The method presented requires the node values at the 'output' node and the
immediately adjacent nodes to be stored.
It uses these values to calculate a velocity vector at the output node for each
time-step, and then calculating the 'intensity' at this node.
The directional response is found by multiplying the magnitude of the intensity
by the squared magnitude of some polar pattern in the direction of the
intensity.
The output can be split into multiple frequency bands, and a different
polar-pattern used per band, if frequency-dependent output is required.

The main draw-back of this method is that its accuracy is dependent on the
directional error (not dispersion error!) of the underlying mesh, which is much
higher for the tetrahedral mesh (a mean of 8%) than other mesh topologies
(which generally have a mean of less than 1% directional error).

Only one other method for modelling directional receivers is presented in the
literature (see @southern2007_2).
The alternate method is similar, but directly compares pressure values at nodes
placed (approximately) in axial directions, analogous to the Blumlein microphone
technique.
The technique also requires very close node spacings, which it achieves by
oversampling the mesh, which is very computationally expensive.
The method presented by @hacihabiboglu2010 is more versatile and much more
efficient, so the method by @southern2007_2 will not be further discussed here.

### @beeson2007

In this paper the equivalence of the K-DWM and W-DWM models is proved, and
a method for interfacing between the two models, called the *KW pipe*, is
provided.
This is useful in cases where the more flexible properties of the W-DWM with
respect to frequency-dependent scattering are required, but only over a small
portion of the mesh, for example at its boundaries.
In these cases, the majority of the mesh can still be modelled using K-DWM
nodes, which require much less memory and processing time.
It is expected that this method will be used to implement materials and surfaces
in the author's own implementation.

Only the 1D and 2D cases are discussed in this paper, but a generalisation to 3
dimensions should be reasonably straight-forward.

### @southern2011

A method for the combination of room impulse responses (RIRs) generated by
different modelling methods into a single *hybrid* RIR is presented.

Geometric modelling methods, such as ray tracing and image-source modelling,
model higher frequencies well, but are inaccurate when the sizes of the
reflective surfaces are of the same or lower order as the wavelength being
modelled.
Conversely, the DWM can be made accurate across the entire frequency spectrum,
but is too expensive to compute at
higher frequencies, as the complexity of the computation increases cubically
with the maximum frequency required at the output.

This paper proposes a method which could be used to combine outputs from
ray tracing and DWM modelling, producing a RIR which is reasonably accurate
across the spectrum, but also not prohibitively expensive to calculate.

Bibliography (and Future Reading)
=================================

---
nocite: |
    @beeson2004,
    @beeson2007,
    @bilbao2013,
    @campos2000,
    @duyne,
    @duyne1996,
    @hacihabiboglu2010,
    @hamilton2013,
    @kelloniemi2006,
    @kim2009,
    @miklavcic2014,
    @murphy2000,
    @murphy2007,
    @savioja_nam02,
    @schroeder1965,
    @sheaffer2013,
    @shelley2007,
    @krokstad1968,
...

