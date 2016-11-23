# Packaged HRTF data

The hrtf data files here are taken from the [Ircam Listen database](http://recherche.ircam.fr/equipes/salles/listen/download.html).

The site does not list a copyright notice for these files, but an appropriate
copyright notice will be added here if necessary (or these files can be
removed).

Only one set of compensated data is stored here.
To use a different database, add it here, ensure the naming conventions of the
files follow the Listen naming format, and update the appropriate
`CMakeLists.txt`.

# HRTF naming convention:

HRTF files shall conform to the following regular expression:

```
.*R([0-9]+)_T([0-9]+)_P([0-9]+).*
```

* The first capture group is a radial distance in centimetres (currently unused).
* The second capture group is the azimuth in degrees.
    * 0 is straight ahead, 180 is behind
    * 0 to 180 is ahead to behind on the left
    * 180 to 360 is behind to ahead on the right
* The third capture group is the elevation in degrees.
    * 0 is level with the listener
    * 0 to 90 is ahead to above
    * 90 to 270 is redundant
    * 270 to 360 is below to ahead
