#!/usr/local/bin/python

import argparse
import soundfile
import numpy
import matplotlib.pyplot as plt


def schroder_integral(sig):
    return numpy.cumsum(numpy.square(sig[::-1]))[::-1]


def p2db(p):
    return 10.0 * numpy.log10(p)


def db_point(db, point):
    return len(db) - numpy.searchsorted(db[::-1], point, side='left')


def decay_time(db, b_db, e_db, min_db):
    b = db_point(db, b_db)
    e = db_point(db, e_db)

    y = db[b:e]
    x = numpy.arange(b, e)

    A = numpy.vstack([x, numpy.ones(len(x))]).T
    m, c = numpy.linalg.lstsq(A, y)[0]

    r = numpy.arange(0, len(db))

    return (min_db - c) / m


def main():
    parser = argparse.ArgumentParser(description='Calculate RT60.')
    parser.add_argument('file_name', type=str, help='an audio file')

    args = parser.parse_args()

    sig, sr = soundfile.read(args.file_name)
    integral = schroder_integral(sig)
    integral /= integral[0]
    db = p2db(integral)

    t20 = decay_time(db, -5, -25, -60)
    t30 = decay_time(db, -5, -35, -60)

    print 't20:', t20 / sr
    print 't30:', t30 / sr


if __name__ == '__main__':
    main()
