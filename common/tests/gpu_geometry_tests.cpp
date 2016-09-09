#include "common/azimuth_elevation.h"
#include "common/cl/geometry.h"
#include "common/cl/geometry_structs.h"
#include "common/conversions.h"
#include "common/geo/geometric.h"
#include "common/map_to_vector.h"
#include "common/program_wrapper.h"
#include "common/progress_bar.h"

#include "gtest/gtest.h"

#include <random>

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

class program final {
public:
    program(const compute_context& cc)
            : wrapper(cc,
                      std::vector<std::string>{
                              cl_representation_v<volume_type>,
                              cl_representation_v<surface>,
                              cl_representation_v<triangle>,
                              cl_representation_v<triangle_verts>,
                              cl_representation_v<ray>,
                              cl_representation_v<triangle_inter>,
                              cl_representation_v<intersection>,
                              cl_sources::geometry,
                              source}) {}

    auto get_triangle_vert_intersection_test_kernel() const {
        return wrapper.get_kernel<cl::Buffer, cl::Buffer, cl::Buffer>(
                "triangle_vert_intersection_test");
    }

    auto get_ray_triangle_intersection_test_kernel() const {
        return wrapper.get_kernel<cl::Buffer,
                                  cl_uint,
                                  cl::Buffer,
                                  cl::Buffer,
                                  cl::Buffer>("ray_triangle_intersection_test");
    }

private:
    program_wrapper wrapper;
    static constexpr auto source{R"(

kernel void triangle_vert_intersection_test(
        const global triangle_verts* t,
        const global ray* r,
        global triangle_inter* ret) {
    const size_t thread = get_global_id(0);
    ret[thread] = (triangle_inter){};
    const triangle_verts i = t[thread];
    const ray j = r[thread];
    const triangle_inter k = triangle_vert_intersection(i, j);
    ret[thread] = k;
}

kernel void ray_triangle_intersection_test(
        const global triangle* triangles,
        uint num_triangles,
        const global float3* vertices,
        const global ray* rays,
        global intersection* ret) {
    const size_t thread = get_global_id(0);
    ret[thread] = (intersection){};
    const ray i = rays[thread];
    const intersection j = ray_triangle_intersection(i,
                                                     triangles,
                                                     num_triangles,
                                                     vertices,
                                                     ~(uint)(0));
    ret[thread] = j;
}

)"};
};

constexpr const char* program::source;

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

aligned::vector<geo::triangle_vec3> random_triangle_vec3s(size_t num) {
    auto engine{std::default_random_engine{std::random_device{}()}};

    aligned::vector<geo::triangle_vec3> ret;
    ret.reserve(num);
    for (auto i{0u}; i != num; ++i) {
        ret.emplace_back(random_triangle_vec3(engine));
    }
    return ret;
}

aligned::vector<geo::ray> random_rays(size_t num) {
    auto engine{std::default_random_engine{std::random_device{}()}};

    aligned::vector<geo::ray> ret;
    ret.reserve(num);
    for (auto i{0u}; i != num; ++i) {
        ret.emplace_back(random_ray(engine));
    }
    return ret;
}

TEST(gpu_geometry, triangle_vert_intersection) {
    const auto num_tests{10000};

    const auto triangle_verts{random_triangle_vec3s(num_tests)};
    const auto rays{random_rays(num_tests)};

    const compute_context cc{};

    const program prog{cc};
    auto kernel{prog.get_triangle_vert_intersection_test_kernel()};

    cl::CommandQueue queue{cc.context, cc.device};

    const auto triangle_verts_buffer{load_to_buffer(
            cc.context,
            map_to_vector(triangle_verts,
                          [](const auto& i) { return convert(i); }),
            true)};

    const auto rays_buffer{load_to_buffer(
            cc.context,
            map_to_vector(rays, [](const auto& i) { return convert(i); }),
            true)};

    cl::Buffer triangle_inter_buffer{
            cc.context, CL_MEM_READ_WRITE, sizeof(triangle_inter) * num_tests};

    kernel(cl::EnqueueArgs(queue, cl::NDRange(num_tests)),
           triangle_verts_buffer,
           rays_buffer,
           triangle_inter_buffer);

    const auto gpu_ret{
            read_from_buffer<triangle_inter>(queue, triangle_inter_buffer)};

    ASSERT_TRUE(proc::any_of(gpu_ret,
                             [](const auto& i) { return i.t || i.u || i.v; }));

    for (auto i{0u}; i != num_tests; ++i) {
        const auto inter{
                geo::triangle_intersection(triangle_verts[i], rays[i])};

        if (inter) {
            ASSERT_TRUE(almost_equal(gpu_ret[i].t, inter ? inter->t : 0, 1));
            ASSERT_TRUE(almost_equal(gpu_ret[i].u, inter ? inter->u : 0, 1));
            ASSERT_TRUE(almost_equal(gpu_ret[i].v, inter ? inter->v : 0, 1));
        }
    }
}

std::tuple<aligned::vector<glm::vec3>, aligned::vector<triangle>>
random_triangles(size_t num) {
    std::default_random_engine engine{std::random_device{}()};

    aligned::vector<glm::vec3> vertices;
    vertices.reserve(num * 3);
    aligned::vector<triangle> triangles;
    triangles.reserve(num);

    for (auto i{0u}; i != num; ++i) {
        for (auto j{0u}; j != 3; ++j) {
            vertices.emplace_back(random_unit_vector(engine));
        }
        triangles.emplace_back(triangle{0, i * 3 + 0, i * 3 + 1, i * 3 + 2});
    }

    return std::make_tuple(vertices, triangles);
}

TEST(gpu_geometry, ray_triangle_intersection) {
    const auto num_tests{10000};
    const auto num_triangles{1000};

    const auto triangles{random_triangles(num_triangles)};
    const auto rays{random_rays(num_tests)};

    const compute_context cc{};

    const program prog{cc};
    auto kernel{prog.get_ray_triangle_intersection_test_kernel()};

    cl::CommandQueue queue{cc.context, cc.device};

    const auto triangles_buffer{
            load_to_buffer(cc.context, std::get<1>(triangles), true)};

    const auto vertices_buffer{load_to_buffer(
            cc.context,
            map_to_vector(std::get<0>(triangles),
                          [](const auto& i) { return to_cl_float3(i); }),
            true)};

    const auto rays_buffer{load_to_buffer(
            cc.context,
            map_to_vector(rays, [](const auto& i) { return convert(i); }),
            true)};

    cl::Buffer intersections_buffer{
            cc.context, CL_MEM_READ_WRITE, sizeof(intersection) * num_tests};

    kernel(cl::EnqueueArgs(queue, cl::NDRange(num_tests)),
           triangles_buffer,
           num_triangles,
           vertices_buffer,
           rays_buffer,
           intersections_buffer);

    const auto gpu_intersections{
            read_from_buffer<intersection>(queue, intersections_buffer)};

    auto pb{progress_bar{std::cout, num_tests}};
    for (auto i{0u}; i != num_tests; ++i, pb += 1) {
        const auto inter{geo::ray_triangle_intersection(
                rays[i], std::get<1>(triangles), std::get<0>(triangles))};
        if (inter) {
            ASSERT_EQ(gpu_intersections[i].index, inter->index);
            ASSERT_TRUE(almost_equal(
                    gpu_intersections[i].inter.t, inter->inter.t, 10));
            ASSERT_TRUE(almost_equal(
                    gpu_intersections[i].inter.u, inter->inter.u, 10));
            ASSERT_TRUE(almost_equal(
                    gpu_intersections[i].inter.v, inter->inter.v, 10));
        }
    }
}
