#!/usr/bin/env python

import argparse, sys, re


def remove_dif(text):
    return re.sub(
            r'\\DIF[^\\]+?{(.*?)}',
            lambda m: m.group(1),
            text,
            flags=re.MULTILINE | re.DOTALL)


def convert(file, inplace):
    with open(file) as f:
        old = f.read()

    new = re.sub(
            r'\\makeatletter(.*?)\\makeatother',
            lambda m: r'\makeatletter{}\makeatother'.format(
                remove_dif(m.group(1))),
            old,
            flags=re.MULTILINE | re.DOTALL)

    if inplace:
        with open(file, 'w') as f:
            f.write(new)
    else:
        sys.stdout.write(new)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('file')
    parser.add_argument('-i', '--inplace', action='store_true')
    args = parser.parse_args()
    convert(args.file, args.inplace)
    return 0


if __name__ == '__main__':
    sys.exit(main())
