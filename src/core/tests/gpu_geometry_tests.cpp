#include "core/azimuth_elevation.h"
#include "core/cl/geometry.h"
#include "core/cl/geometry_structs.h"
#include "core/conversions.h"
#include "core/geo/geometric.h"
#include "core/program_wrapper.h"

#include "utilities/map_to_vector.h"
#include "utilities/mapping_iterator_adapter.h"
#include "utilities/progress_bar.h"

#include "gtest/gtest.h"

#include <random>

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

class program final {
public:
    program(const compute_context& cc)
            : wrapper_{cc,
                       std::vector<std::string>{
                               cl_representation_v<bands_type>,
                               cl_representation_v<surface<simulation_bands>>,
                               cl_representation_v<triangle>,
                               cl_representation_v<triangle_verts>,
                               cl_representation_v<ray>,
                               cl_representation_v<triangle_inter>,
                               cl_representation_v<intersection>,
                               cl_sources::geometry,
                               source_}} {}

    auto get_triangle_vert_intersection_test_kernel() const {
        return wrapper_.get_kernel<cl::Buffer, cl::Buffer, cl::Buffer>(
                "triangle_vert_intersection_test");
    }

    auto get_ray_triangle_intersection_test_kernel() const {
        return wrapper_.get_kernel<cl::Buffer,
                                   cl_uint,
                                   cl::Buffer,
                                   cl::Buffer,
                                   cl::Buffer>(
                "ray_triangle_intersection_test");
    }

    auto get_line_segment_sphere_intersection_test_kernel() const {
        return wrapper_.get_kernel<cl::Buffer,
                                   cl::Buffer,
                                   cl::Buffer,
                                   cl::Buffer,
                                   cl::Buffer>(
                "line_segment_sphere_intersection_test");
    }

    auto get_line_segment_sphere_percentage_test_kernel() const {
        return wrapper_.get_kernel<cl_float3,
                                   cl::Buffer,
                                   cl_float3,
                                   cl_float,
                                   cl::Buffer>(
                "line_segment_sphere_percentage_test");
    }

private:
    program_wrapper wrapper_;
    static constexpr auto source_ = R"(

kernel void triangle_vert_intersection_test(const global triangle_verts* t,
                                            const global ray* r,
                                            global triangle_inter* ret) {
    const size_t thread = get_global_id(0);
    ret[thread] = (triangle_inter){};
    const triangle_verts i = t[thread];
    const ray j = r[thread];
    const triangle_inter k = triangle_vert_intersection(i, j);
    ret[thread] = k;
}

kernel void ray_triangle_intersection_test(const global triangle* triangles,
                                           uint num_triangles,
                                           const global float3* vertices,
                                           const global ray* rays,
                                           global intersection* ret) {
    const size_t thread = get_global_id(0);
    ret[thread] = (intersection){};
    const ray i = rays[thread];
    const intersection j = ray_triangle_intersection(
            i, triangles, num_triangles, vertices, ~(uint)(0));
    ret[thread] = j;
}

kernel void line_segment_sphere_intersection_test(const global float3* p1,
                                                  const global float3* p2,
                                                  const global float3* sc,
                                                  const global float* r,
                                                  global char* ret) {
    const size_t thread = get_global_id(0);
    ret[thread] = line_segment_sphere_intersection(
            p1[thread], p2[thread], sc[thread], r[thread]);
}

kernel void line_segment_sphere_percentage_test(float3 p1,
                                                const global float3* p2,
                                                float3 sc,
                                                float r,
                                                global char* ret) {
    const size_t thread = get_global_id(0);
    ret[thread] = line_segment_sphere_intersection(p1, p2[thread], sc, r);
}

)";
};

constexpr const char* program::source_;

template <typename t>
geo::triangle_vec3 random_triangle_vec3(t& engine) {
    return geo::triangle_vec3{{random_unit_vector(engine),
                               random_unit_vector(engine),
                               random_unit_vector(engine)}};
}

template <typename t>
geo::ray random_ray(t& engine) {
    return geo::ray{glm::vec3{0}, random_unit_vector(engine)};
}

util::aligned::vector<geo::triangle_vec3> random_triangle_vec3s(size_t num) {
    auto engine = std::default_random_engine{std::random_device{}()};

    util::aligned::vector<geo::triangle_vec3> ret;
    ret.reserve(num);
    for (auto i = 0u; i != num; ++i) {
        ret.emplace_back(random_triangle_vec3(engine));
    }
    return ret;
}

util::aligned::vector<geo::ray> random_rays(size_t num) {
    auto engine = std::default_random_engine{std::random_device{}()};

    util::aligned::vector<geo::ray> ret;
    ret.reserve(num);
    for (auto i = 0u; i != num; ++i) {
        ret.emplace_back(random_ray(engine));
    }
    return ret;
}

TEST(gpu_geometry, triangle_vert_intersection) {
    const auto num_tests = 10000;

    const auto triangle_verts = random_triangle_vec3s(num_tests);
    const auto rays = random_rays(num_tests);

    const compute_context cc{};

    const program prog{cc};
    auto kernel = prog.get_triangle_vert_intersection_test_kernel();

    cl::CommandQueue queue{cc.context, cc.device};

    const auto triangle_verts_buffer = load_to_buffer(
            cc.context,
            util::map_to_vector(begin(triangle_verts),
                                end(triangle_verts),
                                [](const auto& i) { return convert(i); }),
            true);

    const auto rays_buffer = load_to_buffer(
            cc.context,
            util::map_to_vector(begin(rays),
                                end(rays),
                                [](const auto& i) { return convert(i); }),
            true);

    cl::Buffer triangle_inter_buffer{
            cc.context, CL_MEM_READ_WRITE, sizeof(triangle_inter) * num_tests};

    kernel(cl::EnqueueArgs(queue, cl::NDRange(num_tests)),
           triangle_verts_buffer,
           rays_buffer,
           triangle_inter_buffer);

    const auto gpu_ret =
            read_from_buffer<triangle_inter>(queue, triangle_inter_buffer);

    ASSERT_TRUE(std::any_of(begin(gpu_ret), end(gpu_ret), [](const auto& i) {
        return i.t || i.u || i.v;
    }));

    for (auto i = 0u; i != num_tests; ++i) {
        const auto inter =
                geo::triangle_intersection(triangle_verts[i], rays[i]);

        if (inter) {
            ASSERT_TRUE(almost_equal(gpu_ret[i].t, inter ? inter->t : 0, 1));
            ASSERT_TRUE(almost_equal(gpu_ret[i].u, inter ? inter->u : 0, 1));
            ASSERT_TRUE(almost_equal(gpu_ret[i].v, inter ? inter->v : 0, 1));
        }
    }
}

struct really_basic_scene final {
    util::aligned::vector<glm::vec3> vertices;
    util::aligned::vector<triangle> triangles;
};

really_basic_scene random_triangles(size_t num) {
    std::default_random_engine engine{std::random_device{}()};

    util::aligned::vector<glm::vec3> vertices;
    vertices.reserve(num * 3);
    util::aligned::vector<triangle> triangles;
    triangles.reserve(num);

    for (auto i = 0u; i != num; ++i) {
        for (auto j = 0u; j != 3; ++j) {
            vertices.emplace_back(random_unit_vector(engine));
        }
        triangles.emplace_back(triangle{0, i * 3 + 0, i * 3 + 1, i * 3 + 2});
    }

    return {vertices, triangles};
}

TEST(gpu_geometry, ray_triangle_intersection) {
    const auto num_tests = 10000;
    const auto num_triangles = 1000;

    const auto scene = random_triangles(num_triangles);
    const auto rays = random_rays(num_tests);

    const compute_context cc{};

    const program prog{cc};
    auto kernel = prog.get_ray_triangle_intersection_test_kernel();
    cl::CommandQueue queue{cc.context, cc.device};

    const auto triangles_buffer =
            load_to_buffer(cc.context, scene.triangles, true);

    const auto vertices_buffer = load_to_buffer(
            cc.context,
            util::map_to_vector(begin(scene.vertices),
                                end(scene.vertices),
                                [](const auto& i) { return to_cl_float3(i); }),
            true);

    const auto rays_buffer = load_to_buffer(
            cc.context,
            util::map_to_vector(begin(rays),
                                end(rays),
                                [](const auto& i) { return convert(i); }),
            true);

    cl::Buffer intersections_buffer{
            cc.context, CL_MEM_READ_WRITE, sizeof(intersection) * num_tests};

    kernel(cl::EnqueueArgs(queue, cl::NDRange(num_tests)),
           triangles_buffer,
           num_triangles,
           vertices_buffer,
           rays_buffer,
           intersections_buffer);

    const auto gpu_intersections =
            read_from_buffer<intersection>(queue, intersections_buffer);

    util::progress_bar pb;
    for (auto i = 0u; i != num_tests; ++i) {
        const auto inter = geo::ray_triangle_intersection(
                rays[i], scene.triangles, scene.vertices);
        if (inter) {
            ASSERT_EQ(gpu_intersections[i].index, inter->index);
            ASSERT_TRUE(almost_equal(
                    gpu_intersections[i].inter.t, inter->inter.t, 10));
            ASSERT_TRUE(almost_equal(
                    gpu_intersections[i].inter.u, inter->inter.u, 10));
            ASSERT_TRUE(almost_equal(
                    gpu_intersections[i].inter.v, inter->inter.v, 10));
        }

        set_progress(pb, i, num_tests);
    }
}

TEST(gpu_geometry, line_sphere_intersection) {
    //  preamble
    const compute_context cc{};
    const program prog{cc};
    auto kernel = prog.get_line_segment_sphere_intersection_test_kernel();
    cl::CommandQueue queue{cc.context, cc.device};

    //  set up test cases
    struct test_case final {
        cl_float3 p1;
        cl_float3 p2;
        cl_float3 sc;
        cl_float r;
        bool expected_result;
    };

    util::aligned::vector<test_case> test_cases{

            {{{-1, 0, 0}}, {{1, 0, 0}}, {{0, 0, 0}}, 0.1, true},
            {{{0, -1, 0}}, {{0, 1, 0}}, {{0, 0, 0}}, 0.1, true},
            {{{0, 0, -1}}, {{0, 0, 1}}, {{0, 0, 0}}, 0.1, true},

            {{{-1, 0, 0}}, {{1, 0, 0}}, {{0, 1, 0}}, 0.1, false},
            {{{-1, 0, 0}}, {{1, 0, 0}}, {{0, -1, 0}}, 0.1, false},

            {{{-1, 0, 0}}, {{1, 0, 0}}, {{0, 0, 0}}, 10, true},

            {{{0, 0, 0}}, {{100, 0, 0}}, {{1.5, 0, 0}}, 1, true},
            {{{0, 0, 0}}, {{-100, 0, 0}}, {{1.5, 0, 0}}, 1, false},

            {{{0, 0, 0}}, {{100, 0, 0}}, {{0, 2, 0}}, 1.9, false},
            {{{0, 0, 0}}, {{100, 0, 0}}, {{0, 2, 0}}, 2.1, true},

    };

    //  set up buffers
    const auto gen_buffer = [&](const auto& lambda) {
        return load_to_buffer(
                cc.context,
                util::map_to_vector(begin(test_cases), end(test_cases), lambda),
                true);
    };

    const auto p1_buffer = gen_buffer([](const auto& i) { return i.p1; });
    const auto p2_buffer = gen_buffer([](const auto& i) { return i.p2; });
    const auto sc_buffer = gen_buffer([](const auto& i) { return i.sc; });
    const auto r_buffer = gen_buffer([](const auto& i) { return i.r; });
    cl::Buffer ret_buffer{
            cc.context, CL_MEM_READ_WRITE, sizeof(cl_char) * test_cases.size()};

    kernel(cl::EnqueueArgs{queue, cl::NDRange{test_cases.size()}},
           p1_buffer,
           p2_buffer,
           sc_buffer,
           r_buffer,
           ret_buffer);

    const auto results = read_from_buffer<cl_char>(queue, ret_buffer);

    for (auto i = 0ul, e = test_cases.size(); i != e; ++i) {
        ASSERT_EQ(test_cases[i].expected_result, results[i]);
    }
}

TEST(gpu_geometry, line_sphere_percentage) {
    const compute_context cc{};
    const program prog{cc};
    auto kernel = prog.get_line_segment_sphere_percentage_test_kernel();
    cl::CommandQueue queue{cc.context, cc.device};

    std::default_random_engine engine{std::random_device{}()};
    std::uniform_real_distribution<float> distance_dist{1.0f, 10.0f};
    std::uniform_real_distribution<float> radius_dist{0.1f, 1.0f};

    constexpr auto tests = 1 << 20;

    for (auto i = 0ul; i != 100; ++i) {
        const auto sphere_distance = distance_dist(engine);
        const auto sphere_radius = radius_dist(engine);

        const auto expected_proportion =
                (1 - cos(asin(sphere_radius / sphere_distance))) / 2;

        const auto directions = get_random_directions(tests);
        const auto p2_buffer = load_to_buffer(
                cc.context,
                util::map_to_vector(
                        begin(directions),
                        end(directions),
                        [&](const auto& i) {
                            const auto line_length{
                                    2 * (sphere_distance + sphere_radius)};
                            return to_cl_float3(i * line_length);
                        }),
                true);

        cl::Buffer ret_buffer{
                cc.context, CL_MEM_READ_WRITE, sizeof(cl_char) * tests};

        kernel(cl::EnqueueArgs{queue, cl::NDRange{tests}},
               cl_float3{{0, 0, 0}},
               p2_buffer,
               cl_float3{{sphere_distance, 0, 0}},
               sphere_radius,
               ret_buffer);

        const auto results = read_from_buffer<cl_char>(queue, ret_buffer);
        const auto intersections = std::count_if(
                begin(results), end(results), [](auto i) { return i; });

        const auto found_proportion =
                intersections / static_cast<double>(tests);

        ASSERT_NEAR(expected_proportion, found_proportion, 0.01);
    }
}
