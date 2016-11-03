mkdir -p build
cd build
if cmake .. ; then

export CL_LOG_ERRORS=stdout
#export GTEST_FILTER="*tri_cube_tests*"
cmake --build . && ctest -V

#cmake --build . && cd utils/image_source_comparison && ./image_source_comparison

#cmake --build . && cd utils/southern2013_2_cuboid && ./southern2013_2_cuboid | tee output.txt

#cmake --build . && ctest -V && callraytrace vault vault vault
#cmake --build . && ./tests/hybrid_test/hybrid_test ../tests/hybrid_test/output
#cmake --build . && ./impulse_gen/impulse_gen

#cmake --build . && cd ../utils/mic_test && ./run_and_graph.sh

#cmake --build . && ./tests/mesh_impulse_response/write_compensation_signal

#cmake --build . && cd utils/solution_growth && ./solution_growth

#cmake --build . && cd utils/sheaffer2014 && ./sheaffer2014

#cmake --build . && cd utils/waveguide_distance_test && ./waveguide_distance_test

#cmake --build . && cd utils/diffuse_decay && ./diffuse_decay

#cmake --build . && cd utils/southern2011 && ./southern2011
#cmake --build . && cd utils/siltanen2013 && ./siltanen2013

#cmake --build . && cd ../utils/boundary_test && ./run_and_graph.sh

#cmake --build . && cd ../utils/fitted_boundary && ./run_and_graph.sh

fi
