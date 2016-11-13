important
=========

Sanity checks - do big echo-y rooms sound big and echo-y? Do small rooms sound
small? Can I get the correct direct/reflected ratios for near and far sources?

Put a really short hanning window on the beginning of output signals to hide
dc drift? QUALITY ENGINEERING

Threaded engine - finish completely

Not convinced that waveguide/raytracer levels are matched properly.

Look again at frequency domain test?

app stuff
---------

Finish adding the help panel info.

Load/save material + receiver presets.
    Or at least have some good ones compiled-in

Close and save commands.

Clipping planes.

Move node colour processing to vertex shader(s).

Use scoped connections to connect debug processing callbacks until they are
first called.

Use appropriate surfaces for newly-loaded models
    i.e. make sure default constructor for 'material' is sensible

Display reason for quitting if the engine quits early.

Improve the source/receiver editors.

Model View
    Pan.

    Reset view.

    Make sources and receivers draggable.

        Highlight when hovered/moved?

        Make them visible

    Improve source/reciever + microphone direction models.

Fail for really long reverbs i.e. longer than 60 seconds.
    OR disable absorptions lower than (for example) 0.01

Benchmark gpu vs cpu performance.

Window to configure output samplerate/bit-depth before rendering.

documentation
-------------

Compare to previous results from undergrad final commit. Compare code too?

Set a bunch of listening tasks.
    Connect expectations of the algorithm and compare them to the outputs.
    Match the audible output to the changes in the code, where possible.

normal
======

Ambisonic polar patterns for microphones.

Fuzzing.

Reduce visiblity of methods wherever possible.

nice-to-have
============

Speed up compile times.

Can I use something like a spatial sinc kernel to place the initial impulse at
an exact location?

Directional sources.

Real-time simulation.

Built-in auralization.

Use Boost Units to validate physical quantities.

Look into other approaches for microphone modelling, which don't affect modal
response as much.

Fix high memory usage when constructing mesh, i.e. when memory usage doubles
occasionally due to copying stuff around.

Search for all divides, ensure the denominator can never be zero.

Closest triangle algo is really dumb + slow.
Fine for the time being, might be a bottleneck in the future.

Better visualisation colour map.

Call from the commandline.

Save/load presets from file.

Variable speed of sound.

Undo/redo.

Correlation meter?
View - add info panel, correlation meter

Use proper BRDF instead of lambertian diffusion.

Trace properly in non-closed scenes.
Possible with anechoic waveguide boundaries?
Probably not because 'ghost points' would end up coinciding with air points.

Air absorption - Better calculation method based on temperature, humidity,
ambient density, speed of sound.
Implement as a post-process in all models.
