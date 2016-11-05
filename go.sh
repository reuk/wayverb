#!/bin/sh

export CL_LOG_ERRORS=stdout
#export GTEST_FILTER="*tri_cube_tests*"

if ./build.sh ; then
#if ./build.sh 2>&1 | tee build_log.txt ; then
cd build
ctest -V

#cd utils/image_source_comparison && ./image_source_comparison

#cd utils/southern2013_2_cuboid && ./southern2013_2_cuboid | tee output.txt

#ctest -V && callraytrace vault vault vault
#./tests/hybrid_test/hybrid_test ../tests/hybrid_test/output
#./impulse_gen/impulse_gen

#cd ../utils/mic_test && ./run_and_graph.sh

#./tests/mesh_impulse_response/write_compensation_signal

#cd utils/solution_growth && ./solution_growth

#cd utils/sheaffer2014 && ./sheaffer2014

#cd utils/waveguide_distance_test && ./waveguide_distance_test

#cd utils/diffuse_decay && ./diffuse_decay

#cd utils/southern2011 && ./southern2011
#cd utils/siltanen2013 && ./siltanen2013

#cd ../utils/boundary_test && ./run_and_graph.sh

#cd ../utils/fitted_boundary && ./run_and_graph.sh

fi
