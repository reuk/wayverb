# TODO

## critical

check raytracer ongoing energy
    if direction is scattering-weighted, should the ongoing energy be weighted too?
    what does this mean for diffuse rain?

Raytracer is slower when waveguide cutoff is high???

Combine fields in condensed_node struct.

Is it worth checking all paths in the image source tree?

Soft source without solution growth.

Only ambisonic output?

### app stuff

Finish adding the help panel info.
    switch to normal tooltips

Benchmark gpu vs cpu performance.

### documentation

Compare to previous results from undergrad final commit. Compare code too?

Set a bunch of listening tasks.
    Connect expectations of the algorithm and compare them to the outputs.
    Match the audible output to the changes in the code, where possible.

## normal

Fuzzing.
    File loading is especially likely to be vulnerable.

## nice-to-have

### app

Improve source/reciever + microphone direction models.

Built-in auralization.

Better visualisation colour map.

Save/load presets from file.

Variable speed of sound.

Undo/redo.

Better comparative visualisation of rays + waves

### library

Speed up compile times.

Can I use something like a spatial sinc kernel to place the initial impulse at
an exact location?

Directional sources.

Real-time simulation.

Ambisonic output/microphone capsules.

Use Boost Units to validate physical quantities.

Look into other approaches for microphone modelling, which don't affect modal
response as much.

Closest triangle algo is really dumb + slow. It's fine for the time being, but
might be a bottleneck in the future.

Call from the commandline.

Use proper BRDF instead of lambertian diffusion.

Trace properly in non-closed scenes.
Possible with anechoic waveguide boundaries?
Probably not because 'ghost points' would end up coinciding with air points.

Air absorption - Better calculation method based on temperature, humidity,
ambient density, speed of sound.
Implement as a post-process in all models.

Automatically align waveguide mesh to model's minimal bounding box.
