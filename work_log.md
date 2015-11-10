notes 2/11/2015
===============

* maybe discuss UI a little more
* how do I make the most of the underlying model
* is the strength of the model
* simply that it's realistic
* or are there additional control mechanisms that I can make available

* we have to see how the project develops, and then think about the best way of
actually doing it

* another direction is to make something that's not necessarily optimal
    * to compare the element methods etc.

* state the *main* research questions in bullets as well as the auxiliary

* in what ways will I have developed additional skills or new ways of
problem solving, by the end of the year?
* UI is one area
* on a technical level, then solving the problems here might also work!
* but maybe have a plan B
* e.g. comparing the element methods, looking for realism

* also consider branching
* a 'real-time' branch vs a super-realistic branch
* or a 'high-quality' render

* sonically, papers are not very useful
* better off listening to stuff myself
* therefore, actually building the stuff

* try to open up several directions I *could* go in

* read!

* concentrate on branching
* rather than having 'one empirical version'
* try working faster, maybe at the expense of code quality etc.

The current problem:
====================

* The mesh sounds weird at high frequencies
    * cause: dispersion error
        * plane waves travel at slightly different speeds depending on their
          frequency and direction relative to the mesh orientation [@duyne1996]

    * solutions:
        * find usable bandwidth of simulation for which error is within some
          tolerable limit, only use this bandwidth
            * will require analysis of dispersion error
            * analysis by @hacihabiboglu2010 suggests that directional error
              for the tetrahedral mesh is significantly greater than the cubic
              mesh (but under what circumstances?)
                * therefore *may not be appropriate* for mic modelling
            * the same paper mentions that the magnitude error of the
              tetrahedral is lowest for the same spatial sampling period
                * but the referenced paper [@hacihabiboglu2008] seems to say
                  otherwise?
        * try to correct the error somehow to increase the usable bandwidth
            * investigate frequency warping to mitigate frequency-dependent
              error

Dispersion analysis
===================

* dispersion analysis is achieved by applying a von neumann analysis directly on
  the FDS. [@duyne]

    * try to find out how to do a dispresion analysis
        * @savioja2000 might be useful

        * take the difference equation for the system
        * consider it to be in continuous space by replacing sample points in
          space with generalized impulse functions
            * I don't understand this
        * take the spatial fourier transform of the difference equation,
          replacing function points with corresponding linear phase terms
            * I don't understand this either
        * this gives us a filter equation with a coefficient
        * we can find the coefficient in terms of the linear phase at the
          waveguide mesh points (I think)
        * we can find the phase distance travelled in one time sample using
          this coefficient

    * do the thing
        * I wrote a python script that replicates the measurements in @duyne
        * but somehow it's not right quite
            * read @warren1998 to get a better idea of how dispersion analysis
              works

    * write a program which finds the maximum allowable bandwidth for a given
      maximum dispersion speed error
