#!/usr/local/bin/python

from subprocess import call
from os.path import join
from os import environ
from math import pi

from paths import *

environ["GLOG_logtostderr"] = "1"

def main():
    for azimuth, elevation in [
        (pi / 4, pi / 4),
        (pi / 3, pi / 3),
        (0, 0),
        (pi / 4, 0),
    ]:
        az_string = '%s' % float('%.4g' % azimuth)
        el_string = '%s' % float('%.4g' % elevation)
        o_dir = join(out_dir, "az_" + az_string + "_el_" + el_string)
        cmd_1 = ["mkdir", "-p", o_dir]
        cmd_2 = [exe, o_dir, str(azimuth), str(elevation)]
        print cmd_1
        call(cmd_1)
        print cmd_2
        call(cmd_2)

if __name__ == "__main__":
    main()
