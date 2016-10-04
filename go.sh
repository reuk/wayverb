mkdir -p build
cd build

export GLOG_logtostderr=1
#export GTEST_FILTER="*attenuate*"

#cmake .. && make && ctest -V
#cmake .. && make && cd utils/image_source_comparison && ./image_source_comparison

#cmake .. && make && cd utils/southern2013_2_cuboid && ./southern2013_2_cuboid | tee output.txt

#cmake .. && make && ctest -V && callraytrace vault vault vault
#cmake .. && make && ./tests/hybrid_test/hybrid_test ../tests/hybrid_test/output
#cmake .. && make && ./impulse_gen/impulse_gen

cmake .. && make && cd ../utils/mic_test && ./run_and_graph.sh

#cmake .. && make && ./tests/mesh_impulse_response/write_compensation_signal

#cmake .. && make && cd utils/solution_growth && ./solution_growth

#cmake .. && make && cd utils/sheaffer2014 && ./sheaffer2014

#cmake .. && make && cd utils/waveguide_distance_test && ./waveguide_distance_test

#cmake .. && make && cd utils/southern2011 && ./southern2011
#cmake .. && make && cd utils/siltanen2013 && ./siltanen2013
