#include "common/indexing.h"

#include "gtest/gtest.h"

TEST(indexing, relative_position) {
    using namespace indexing;
    ASSERT_EQ(relative_position<1>(0), (index_t<1>{0}));
    ASSERT_EQ(relative_position<1>(1), (index_t<1>{1}));
    ASSERT_EQ(relative_position<1>(2), (index_t<1>{0}));
    ASSERT_EQ(relative_position<1>(3), (index_t<1>{1}));

    ASSERT_EQ(relative_position<2>(0), (index_t<2>{0, 0}));
    ASSERT_EQ(relative_position<2>(1), (index_t<2>{1, 0}));
    ASSERT_EQ(relative_position<2>(2), (index_t<2>{0, 1}));
    ASSERT_EQ(relative_position<2>(3), (index_t<2>{1, 1}));
    ASSERT_EQ(relative_position<2>(4), (index_t<2>{0, 0}));
    ASSERT_EQ(relative_position<2>(5), (index_t<2>{1, 0}));
    ASSERT_EQ(relative_position<2>(6), (index_t<2>{0, 1}));
    ASSERT_EQ(relative_position<2>(7), (index_t<2>{1, 1}));

    ASSERT_EQ(relative_position<3>(0), (index_t<3>{0, 0, 0}));
    ASSERT_EQ(relative_position<3>(1), (index_t<3>{1, 0, 0}));
    ASSERT_EQ(relative_position<3>(2), (index_t<3>{0, 1, 0}));
    ASSERT_EQ(relative_position<3>(3), (index_t<3>{1, 1, 0}));
    ASSERT_EQ(relative_position<3>(4), (index_t<3>{0, 0, 1}));
    ASSERT_EQ(relative_position<3>(5), (index_t<3>{1, 0, 1}));
    ASSERT_EQ(relative_position<3>(6), (index_t<3>{0, 1, 1}));
    ASSERT_EQ(relative_position<3>(7), (index_t<3>{1, 1, 1}));
    ASSERT_EQ(relative_position<3>(8), (index_t<3>{0, 0, 0}));
    ASSERT_EQ(relative_position<3>(9), (index_t<3>{1, 0, 0}));
    ASSERT_EQ(relative_position<3>(10), (index_t<3>{0, 1, 0}));
    ASSERT_EQ(relative_position<3>(11), (index_t<3>{1, 1, 0}));
    ASSERT_EQ(relative_position<3>(12), (index_t<3>{0, 0, 1}));
    ASSERT_EQ(relative_position<3>(13), (index_t<3>{1, 0, 1}));
    ASSERT_EQ(relative_position<3>(14), (index_t<3>{0, 1, 1}));
    ASSERT_EQ(relative_position<3>(15), (index_t<3>{1, 1, 1}));
}

TEST(indexing, flatten) {
    using namespace indexing;
    ASSERT_EQ(flatten<1>(index_t<1>{0}, index_t<1>{0}), 0);
    ASSERT_EQ(flatten<1>(index_t<1>{1}, index_t<1>{0}), 1);

    ASSERT_EQ(flatten<2>(index_t<2>{0, 0}, index_t<2>{3, 4}), 0);
    ASSERT_EQ(flatten<2>(index_t<2>{1, 1}, index_t<2>{3, 4}), 5);
    ASSERT_EQ(flatten<2>(index_t<2>{2, 2}, index_t<2>{3, 4}), 10);

    ASSERT_EQ(flatten<3>(index_t<3>{0, 0, 0}, index_t<3>{2, 3, 4}), 0);
    ASSERT_EQ(flatten<3>(index_t<3>{1, 1, 0}, index_t<3>{2, 3, 4}), 16);
    ASSERT_EQ(flatten<3>(index_t<3>{1, 2, 2}, index_t<3>{2, 3, 4}), 22);
}
