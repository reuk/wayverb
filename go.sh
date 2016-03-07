mkdir -p build
cd build

outdir=impulses
mkdir -p $outdir

progname=./cmd/pwaveguide

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

#cmake .. && make && ctest -V && callraytrace vault vault vault
cmake .. && make && ctest -V && ./boundary_test/boundary_reflect ./boundary_test && python ../boundary_test/graphs.py
