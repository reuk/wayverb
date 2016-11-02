#!/usr/local/bin/python

from subprocess import call
from os.path import join
from os import environ
from math import pi

from paths import *

environ["GLOG_logtostderr"] = "1"

def main():
    for pattern in ["omnidirectional", "cardioid", "bidirectional"]:
        o_dir = join(out_dir, pattern)
        cmd_1 = ["mkdir", "-p", o_dir]
        cmd_2 = [exe, o_dir, pattern]
        print cmd_1
        call(cmd_1)
        print cmd_2
        call(cmd_2)

if __name__ == "__main__":
    main()
