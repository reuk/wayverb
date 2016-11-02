#!/usr/local/bin/python

from subprocess import call
from os.path import join

from paths import *

def main():
    call(["mkdir", "-p", out_dir])
    with open(join(out_dir, "coefficients.json"), "w") as f:
        call([exe], stdout=f)

if __name__ == "__main__":
    main()
