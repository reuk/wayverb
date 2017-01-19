We run three different cuboid rooms with increasing volume.  The same materials
(with absorption and scattering of 0.1 across the spectrum) are used for all
models, and the source and receiver are placed 1m apart in the centre of the
room.  We use the waveguide for the bottom 500Hz with a 'usable portion' of
0.6.

We expect to see the following rt60s:

gigantic_cuboid         35      7.5     20      5,250			2,225				(0.161 * 5250) / (0.1 * 2225) 	= 3.7988764045
large_cuboid            12      4       8       384				352					(0.161 * 384) / (0.1 * 352) 	= 1.7563636364
medium_cuboid           4.5     2.5     3.5     39.375			71.5				(0.161 * 39.375) / (0.1 * 71.5) = 0.8866258741
small_cuboid            2       2.5     3       15				37					(0.161 * 15) / (0.1 * 37) 		= 0.6527027027

At the moment, the ray tracer over-estimates the tail length, while the
waveguide seems to die away faster than expected, especially in larger rooms.

- test whether different boundary types give better results.
