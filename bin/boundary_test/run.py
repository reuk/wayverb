#!/usr/local/bin/python

from subprocess import call
from os.path import join
from math import pi

from paths import *

def main():
    call(["mkdir", "-p", out_dir])
    call([exe, out_dir])

if __name__ == "__main__":
    main()
