mkdir -p build
cd build

outdir_0=mic_axis_rotate_output
outdir_1=mic_offset_rotate_output
mkdir -p $outdir_0
mkdir -p $outdir_1
progname_0=./mic_test/mic_axis_rotate
progname_1=./mic_test/mic_offset_rotate

calltest () {
    $progname_0 $outdir_0
#    $progname_1 $outdir_1 omni
#    $progname_1 $outdir_1 cardioid
#    $progname_1 $outdir_1 bidirectional
}

cmake .. && make && ctest -V && calltest && mv *.energies.txt ../python
