important
=========

Sanity checks - do big echo-y rooms sound big and echo-y? Do small rooms sound
small?

Put a really short hanning window on the beginning of output signals to hide
dc drift? QUALITY ENGINEERING

app stuff
---------

Finish adding the help panel info.

Display reason for quitting if the engine quits early.

Improve the source/receiver editors.

Make source and receiver draggable.

Improve source/reciever + microphone direction models.

Change orientation of whole microphone group, with their own up + facing pairs.

Auto-configure wherever possible, but make it clear what the algorithm is doing.

Make sure all outputs from the same run are normalized appropriately, so don't
write out files immediately. Wait until all files have been generated and then
normalize them all.

Fail for really long reverbs i.e. longer than 60 seconds.

Benchmark gpu vs cpu performance.

documentation
-------------

Compare to previous results from undergrad final commit. Compare code too?

Set a bunch of listening tasks.
    Connect expectations of the algorithm and compare them to the outputs.
    Match the audible output to the changes in the code, where possible.

normal
======

Ambisonic polar patterns for microphones.

Make sure all modules are namespaced.

Reduce visiblity of methods wherever possible.

nice-to-have
============

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
