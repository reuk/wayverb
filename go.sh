mkdir -p build
cd build

outdir=impulses
mkdir -p $outdir

progname=./cmd/wayverb

callraytrace () {
    mkdir -p $outdir/$2
    args=" ../demo/assets/configs/$1.json ../demo/assets/test_models/$2.obj ../demo/assets/materials/$3.json $outdir/$2/$2_$1_$3.wav "
    echo $args

    prefix=""
    case 0 in
        1)
            prefix="valgrind"
            ;;
        2)
            prefix="lldb"
            ;;
    esac
    $prefix $progname $args
}

export GLOG_logtostderr=1
#export GTEST_FILTER="*pcs*"

#cmake .. && make && ctest -V
#cmake .. && make && cd utils/image_source_comparison && ./image_source_comparison

#cmake .. && make && cd utils/southern2013_2_cuboid && ./southern2013_2_cuboid | tee output.txt

#cmake .. && make && make doc && ctest -V
#cmake .. && make && ctest -V && callraytrace vault vault vault
#cmake .. && make && ./tests/hybrid_test/hybrid_test ../tests/hybrid_test/output
#cmake .. && make && ./impulse_gen/impulse_gen

#cmake .. && make && cd ../tests/mic_test && python run_and_graph.py

#cmake .. && make && ./tests/mesh_impulse_response/write_compensation_signal

#cmake .. && make && cd utils/solution_growth && ./solution_growth

#cmake .. && make && cd utils/sheaffer2014 && ./sheaffer2014

cmake .. && make && cd utils/waveguide_distance_test && ./waveguide_distance_test
