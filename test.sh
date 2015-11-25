mkdir -p build
cd build

outdir=mic_test_output
mkdir -p $outdir

progname=./mic_test/mic_test

calltest () {
    $progname $outdir
}

cmake .. && make && ctest -V && calltest
