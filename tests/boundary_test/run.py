from subprocess import call
from os.path import join
from os import environ
from math import pi

exe = "/Users/reuben/dev/waveguide/build/tests/boundary_test/boundary_test"
out_dir = "/Users/reuben/dev/waveguide/tests/boundary_test/output"

environ["GLOG_logtostderr"] = "1"

def main():
    for azimuth, elevation in [
        (pi / 4, pi / 4),
        (pi / 3, pi / 3),
        (0, 0),
        (pi / 4, 0),
    ]:
        o_dir = join(out_dir, "az__" + str(azimuth) + "__el__" + str(elevation))
        cmd_1 = ["mkdir", "-p", o_dir]
        cmd_2 = [exe, o_dir, str(azimuth), str(elevation)]
        print cmd_1
        call(cmd_1)
        print cmd_2
        call(cmd_2)

if __name__ == "__main__":
    main()
