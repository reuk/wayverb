#include "raytracer/image_source_tree.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <random>

TEST(multitree, construct_image_source_tree_small) {
    const aligned::vector<aligned::vector<raytracer::path_element>> paths{
            aligned::vector<raytracer::path_element>{
                    raytracer::path_element{0, true},
                    raytracer::path_element{0, true},
                    raytracer::path_element{0, true}},
            aligned::vector<raytracer::path_element>{
                    raytracer::path_element{0, true},
                    raytracer::path_element{1, true},
                    raytracer::path_element{0, true}}};

    const auto tree{raytracer::construct_image_source_tree(paths)};

    ASSERT_EQ(tree.size(), 1);
    ASSERT_EQ(tree.begin()->item_.index, 0);

    ASSERT_EQ(tree.begin()->branches_.size(), 2);
    ASSERT_EQ(tree.begin()->branches_.begin()->item_.index, 0);
    ASSERT_EQ(std::next(tree.begin()->branches_.begin())->item_.index, 1);
}

/*
TEST(multitree, construct_image_source_tree_large) {
    std::default_random_engine engine{std::random_device{}()};
    std::uniform_int_distribution<cl_ulong> distribution(0, 99);

    const auto make_path{[&] {
        aligned::vector<raytracer::path_element> ret{distribution(engine)};
        std::generate(ret.begin(), ret.end(), [&] {
            return raytracer::path_element{distribution(engine), true};
        });
        return ret;
    }};

    aligned::vector<aligned::vector<raytracer::path_element>> paths{100000};
    std::generate(paths.begin(), paths.end(), make_path);
    const auto tree{raytracer::construct_image_source_tree(paths)};
}
*/