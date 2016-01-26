Formulation of Locally Reacting Surfaces in K-DWM Modelling of Acoustic Spaces
==============================================================================

Reuben's Ongoing Concerns-With-The-Paper Scratch Space
------------------------------------------------------

* so far we've only seen formulations for rectilinear grids and planes
    * will this work for other topologies
    * especially if we need to find velocity components normal to the surface
      nodes

FDTD
----

* sound propagation governed by
    * conservation of mass
    * conservation of momentum

* wave equation is given by eliminating particle velocity
    * giving an equation for change in acoustic pressure against time (?)

* equations for the discretized wave equations on rectilinear regular grids are
  given

* Courant number is cT / X where
    * c is the speed of sound
    * T is the time step
    * X is the grid spacing

* in the 3D case, the courant number (stability condition) is
  Courant number <= 1 / sqrt(3)
    * to minimize dispersion, the Courant number cT/X should be set to the
      stability limit

* dispersion equations are given for rectilinear regular grids

* boundary formulations are derived by combining the discretized wave equation
  with a discretized boundary condition
    * updating boundary nodes using the resulting boundary update equation

Locally Reacting Surfaces
-------------------------

* In a locally reacting surface, the normal component of the particle velocity
  at the wall surface depends on the sound pressure in front of the wall.
    * this holds for surfaces that can't propagate waves parallel to the
      boundary surface

* reflectance / planar wave coefficient is given by
    * (g cos(theta) - 1) / (g cos(theta) + 1) where
        * g is the normalized wall impedance Z / rho c where
            * Z is the boundary impedance
            * rho is the air density
            * c is the speed of sound
    * see H. Kuttruff, Room Acoustics p. 38

* this models only locally reacting surfaces (I think) which are the exception
  rather than the rule - most surfaces in fact DO propagate waves parallel to
  the surface

* can't really model anechoic boundaries with this model

Frequency Independent Boundaries
--------------------------------

* by substituting centred finite difference operators into the continuous
  boundary condition equation, we get an equation which can be written to
  describe a 'ghost point' which lies outside the modelled space

* this equation can be used to eliminate the ghost point in the discretized
  wave equation
    * giving us an update equation for the boundary

* to find an update equation for the corner, recognize that the corner boundary
  must satisfy the boundary conditions of the two joining edges simultaneously
    * we find two ghost point equations
    * then eliminate the ghost points in the discretized wave equation
    * this gives us the corner update equation

Frequency Dependent Boundaries
------------------------------

* replace real-valued wall impedance with complex-valued frequency-dependent
  impedance

* combining impedance equations for mass-like and spring-like boundaries gives
  a single general boundary impedance equation

* insert the impedance equation into the continuous wave equation
    * then use finite difference operators to produce a discretized equation

* the continuous equation includes a second-order derivative and an integral
    * versions of the bilinear transform are used to remove these

* now we use the discretized boundary equation to eliminate the ghost point in
 the discretized wave equation
