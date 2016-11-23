# TODO

## critical

Not convinced that waveguide/raytracer levels are matched properly.

Waveguide growth issue - see: vault model

## important

Sanity checks - do big echo-y rooms sound big and echo-y? Do small rooms sound
small? Can I get the correct direct/reflected ratios for near and far sources?

Put a really short hanning window on the beginning of output signals to hide
dc drift? QUALITY ENGINEERING

Look again at frequency domain test?

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

Reduce visiblity of methods wherever possible.

## nice-to-have

### app

Improve source/reciever + microphone direction models.

Built-in auralization.

Better visualisation colour map.

Save/load presets from file.

Variable speed of sound.

Undo/redo.

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
