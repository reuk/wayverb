mkdir -p build
cd build
if cmake .. ; then

export CL_LOG_ERRORS=stdout
#export GTEST_FILTER="*multiband*"
#make && ctest -V

#make && cd utils/image_source_comparison && ./image_source_comparison

#make && cd utils/southern2013_2_cuboid && ./southern2013_2_cuboid | tee output.txt

#make && ctest -V && callraytrace vault vault vault
#make && ./tests/hybrid_test/hybrid_test ../tests/hybrid_test/output
#make && ./impulse_gen/impulse_gen

#make && cd ../utils/mic_test && ./run_and_graph.sh

#make && ./tests/mesh_impulse_response/write_compensation_signal

#make && cd utils/solution_growth && ./solution_growth

#make && cd utils/sheaffer2014 && ./sheaffer2014

#make && cd utils/waveguide_distance_test && ./waveguide_distance_test

#make && cd utils/diffuse_decay && ./diffuse_decay

#make && cd utils/southern2011 && ./southern2011
make && cd utils/siltanen2013 && ./siltanen2013

#make && cd ../utils/boundary_test && ./run_and_graph.sh

#make && cd ../utils/fitted_boundary && ./run_and_graph.sh

fi
