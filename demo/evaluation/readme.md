# Layout

* `object` - files for use with Wayverb.
* `sketchup` - original files, used to generate those in `object`. (Wayverb
  can't read these directly.)

# Exporting for Use in Wayverb

Wayverb interprets the units in the file as metres. Make sure your models are
to scale before attempting to load and render them in Wayverb.

If exporting from Sketchup, use the .obj exporter.  Make sure that "Triangulate
all faces" and "Swap YZ coordinates" are selected, and set "Units" to "Meters".

![These are the preferred Sketchup export options.](export_options.png)

Your model *must* be solid and watertight, without holes or no-thickness
planes.  The waveguide mesh setup process must be able to work out whether each
node is 'inside' or 'outside' the space, and it will not be able to do so if
the model does not have a well-defined inside and outside.

To ensure that your model is valid, you can:

    * Open the model in Sketchup.
    * Select-all and Edit > Make Group.
    * Check the info window (Window > Entity Info).
    * If this window displays a volume, it is correct.

If the model is not valid, you can debug it using the ['Solid Inspector'
plugin](https://extensions.sketchup.com/en/content/solid-inspector).

These instructions are taken from the readme for
[ParallelFDTD](https://github.com/juuli/ParallelFDTD), which uses a similar
(but not idential) technique to Wayverb for setting up a waveguide mesh.

# Model info

-----------------------------------------------------------------------------------------------------------
                        measurements / m
model name              -----------------------	volume / m^3	surface area / m^2 	rt60 at 0.1 absorption
                        x       y       z       
----------------------- ------- ------- ------- --------------- ------------------- -----------------------
bedroom                 4       3       6       
concert                 32.84   15.41   50.71   
echo_tunnel             4       7       100     2,800			2,256				(0.161 * 2800) / (0.1 * 2256) 	= 1.9982269504
gigantic_cuboid         35      7.5     20      5,250			2,225				(0.161 * 5250) / (0.1 * 2225) 	= 3.7988764045
large_cuboid            12      4       8       384				352					(0.161 * 384) / (0.1 * 352) 	= 1.7563636364
medium_cuboid           4.5     2.5     3.5     39.375			71.5				(0.161 * 39.375) / (0.1 * 71.5) = 0.8866258741
small_cuboid            2       2.5     3       15				37					(0.161 * 15) / (0.1 * 37) 		= 0.6527027027
vault                   19      3.6     15.2    
-----------------------------------------------------------------------------------------------------------
