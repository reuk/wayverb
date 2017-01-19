The receiver is placed at the centre of a 100m tunnel, with a source placed
40m away. Surfaces have low absorption (TODO).

We expect to see a direct contribution after 0.118s, and a reflection from the
wall behind the source after 0.176s. There should be further strong reflections
every 0.294s after each of these, as the wave-fronts bounce back and forth
between the ends of the tunnel.

For an accurate tail response, this models needs lots of rays. The walls are
very reflective, which means the rays are traced to a great depth, so it takes
a long time to run.

The results do not show the large echoes that were expected, but there are still
obvious increases in energy every 0.3s.

The tunnel_near test places the source much closer to the receiver, so that
reflections from either end of the tunnel should coincide, producing stronger,
more obvious echoes.  In this test we still expect to see echoes every 0.3s,
but they should have greater amplitude relative to the scattered "base".
