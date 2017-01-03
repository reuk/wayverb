#!/bin/sh

export CL_LOG_ERRORS=stdout
#export GTEST_FILTER="*image_source*"
#export GTEST_FILTER="*app_model*:*round_trip*"

if ./build.sh ; then
#if ./build.sh 2>&1 | tee build_log.txt ; then
cd build

#ctest -V

#cd bin/image_source_comparison && ./image_source_comparison

#cd bin/southern2013_2_cuboid && ./southern2013_2_cuboid | tee output.txt

#ctest -V && callraytrace vault vault vault
#./tests/hybrid_test/hybrid_test ../tests/hybrid_test/output
#./impulse_gen/impulse_gen

#cd ../bin/mic_test && ./run_and_graph.sh

#./tests/mesh_impulse_response/write_compensation_signal

#cd bin/solution_growth && ./solution_growth

#cd bin/sheaffer2014 && ./sheaffer2014

#cd bin/waveguide_distance_test && ./waveguide_distance_test

#cd bin/diffuse_decay && ./diffuse_decay

#cd bin/southern2011 && ./southern2011
cd bin/siltanen2013 && ./siltanen2013
#cd bin/level_match && ./level_match ~/development/waveguide/demo/assets/test_models/vault.obj

#cd ../bin/boundary_test && ./run_and_graph.sh

#cd ../bin/fitted_boundary && ./run_and_graph.sh

fi
