#include "raytracer/image_source/tree.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <random>

TEST(multitree, construct_image_source_tree_small) {
    const util::aligned::vector<
            util::aligned::vector<raytracer::image_source::path_element>>
            paths{util::aligned::vector<raytracer::image_source::path_element>{
                          raytracer::image_source::path_element{0, true},
                          raytracer::image_source::path_element{0, true},
                          raytracer::image_source::path_element{0, true}},
                  util::aligned::vector<raytracer::image_source::path_element>{
                          raytracer::image_source::path_element{0, true},
                          raytracer::image_source::path_element{1, true},
                          raytracer::image_source::path_element{0, true}}};

    raytracer::image_source::tree ist{};
    for (const auto& path : paths) {
        ist.push(path);
    }
    const auto& tree{ist.get_branches()};

    ASSERT_EQ(tree.size(), 1);
    ASSERT_EQ(tree.begin()->item.index, 0);

    ASSERT_EQ(tree.begin()->branches.size(), 2);
    ASSERT_EQ(tree.begin()->branches.begin()->item.index, 0);
    ASSERT_EQ(std::next(tree.begin()->branches.begin())->item.index, 1);
}

TEST(multitree, construct_image_source_tree_large) {
    std::default_random_engine engine{std::random_device{}()};
    std::uniform_int_distribution<cl_uint> distribution{0, 99};

    const auto make_path{[&] {
        util::aligned::vector<raytracer::image_source::path_element> ret{
                distribution(engine)};
        std::generate(ret.begin(), ret.end(), [&] {
            return raytracer::image_source::path_element{distribution(engine),
                                                         true};
        });
        return ret;
    }};

    util::aligned::vector<
            util::aligned::vector<raytracer::image_source::path_element>>
            paths{100000};
    std::generate(paths.begin(), paths.end(), make_path);
    raytracer::image_source::tree tree{};
    for (const auto& path : paths) {
        tree.push(path);
    }
}
