#!/bin/sh

# Run this from the supplied vagrant box so that all the dependencies are
# accounted-for.

# Generate automatic doxygen documentation.
mkdir -p doxy_working
cd doxy_working
cmake -DONLY_BUILD_DOCS=ON .. && cmake --build .
cd -
rm -rf doxy_working

# Generate hand-written documentation.

cd docs_source

# Generate pdf.
make clean
make

# Generate website.
# Github pages will serve the static site from docs, on the master branch.
jekyll build --destination ../docs
