#include "common/azimuth_elevation.h"
#include "common/cl/geometry.h"
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
    program(const cl::Context& context, const cl::Device& device)
            : wrapper(context,
                      device,
                      std::vector<std::string>{cl_sources::scene_structs,
                                               cl_sources::geometry,
                                               source}) {}

    auto get_triangle_vert_intersection_test_kernel() const {
        return wrapper.get_kernel<cl::Buffer, cl::Buffer, cl::Buffer>(
                "triangle_vert_intersection_test");
    }

    auto get_ray_triangle_intersection_test_kernel() const {
        return wrapper.get_kernel<cl::Buffer,
                                  cl_ulong,
                                  cl::Buffer,
                                  cl::Buffer,
                                  cl::Buffer>("ray_triangle_intersection_test");
    }

private:
    program_wrapper wrapper;
    static constexpr const char* source{R"(

kernel void triangle_vert_intersection_test(
        const global TriangleVerts* triangle_verts,
        const global Ray* rays,
        global float* distances) {
    const size_t thread = get_global_id(0);
    distances[thread] = 0;
    const TriangleVerts triangle_vert = triangle_verts[thread];
    const Ray ray = rays[thread];
    const float distance = triangle_vert_intersection(triangle_vert, ray);
    distances[thread] = distance;
}

kernel void ray_triangle_intersection_test(
        const global Triangle* triangles,
        ulong num_triangles,
        const global float3* vertices,
        const global Ray* rays,
        global Intersection* intersections) {
    const size_t thread = get_global_id(0);
    intersections[thread] = (Intersection){};
    const Ray ray = rays[thread];
    const Intersection intersection =
            ray_triangle_intersection(ray, triangles, num_triangles, vertices);
    intersections[thread] = intersection;
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
        ret.push_back(random_triangle_vec3(engine));
    }
    return ret;
}

aligned::vector<geo::ray> random_rays(size_t num) {
    auto engine{std::default_random_engine{std::random_device{}()}};

    aligned::vector<geo::ray> ret;
    ret.reserve(num);
    for (auto i{0u}; i != num; ++i) {
        ret.push_back(random_ray(engine));
    }
    return ret;
}

TEST(gpu_geometry, triangle_vert_intersection) {
    const auto num_tests{10000};

    const auto triangle_verts{random_triangle_vec3s(num_tests)};
    const auto rays{random_rays(num_tests)};

    const auto cc{compute_context{}};

    const auto prog{program{cc.get_context(), cc.get_device()}};
    auto kernel{prog.get_triangle_vert_intersection_test_kernel()};

    auto queue{cl::CommandQueue{cc.get_context(), cc.get_device()}};

    const auto triangle_verts_buffer{load_to_buffer(
            cc.get_context(),
            map_to_vector(triangle_verts,
                          [](const auto& i) { return convert(i); }),
            true)};

    const auto rays_buffer{load_to_buffer(
            cc.get_context(),
            map_to_vector(rays, [](const auto& i) { return convert(i); }),
            true)};

    auto distances_buffer{cl::Buffer(
            cc.get_context(), CL_MEM_READ_WRITE, sizeof(cl_float) * num_tests)};

    kernel(cl::EnqueueArgs(queue, cl::NDRange(num_tests)),
           triangle_verts_buffer,
           rays_buffer,
           distances_buffer);

    const auto gpu_distances{
            read_from_buffer<cl_float>(queue, distances_buffer)};

    for (auto i{0u}; i != num_tests; ++i) {
        const auto inter{
                geo::triangle_intersection(triangle_verts[i], rays[i])};

        if (inter) {
            ASSERT_TRUE(
                    almost_equal(gpu_distances[i], inter ? inter->t : 0, 1));
        }
    }
}

std::tuple<aligned::vector<glm::vec3>, aligned::vector<triangle>>
random_triangles(size_t num) {
    auto engine{std::default_random_engine{std::random_device{}()}};

    aligned::vector<glm::vec3> vertices;
    vertices.reserve(num * 3);
    aligned::vector<triangle> triangles;
    triangles.reserve(num);

    for (auto i{0u}; i != num; ++i) {
        for (auto j{0u}; j != 3; ++j) {
            vertices.push_back(random_unit_vector(engine));
        }
        triangles.push_back(triangle{0, i * 3 + 0, i * 3 + 1, i * 3 + 2});
    }

    return std::make_tuple(vertices, triangles);
}

TEST(gpu_geometry, ray_triangle_intersection) {
    const auto num_tests{1000};
    const auto num_triangles{1000};

    const auto triangles{random_triangles(num_triangles)};
    const auto rays{random_rays(num_tests)};

    const auto cc{compute_context{}};

    const auto prog{program{cc.get_context(), cc.get_device()}};
    auto kernel{prog.get_ray_triangle_intersection_test_kernel()};

    auto queue{cl::CommandQueue{cc.get_context(), cc.get_device()}};

    const auto triangles_buffer{
            load_to_buffer(cc.get_context(), std::get<1>(triangles), true)};

    const auto vertices_buffer{load_to_buffer(
            cc.get_context(),
            map_to_vector(std::get<0>(triangles),
                          [](const auto& i) { return to_cl_float3(i); }),
            true)};

    const auto rays_buffer{load_to_buffer(
            cc.get_context(),
            map_to_vector(rays, [](const auto& i) { return convert(i); }),
            true)};

    auto intersections_buffer{cl::Buffer(cc.get_context(),
                                         CL_MEM_READ_WRITE,
                                         sizeof(intersection) * num_tests)};

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
            ASSERT_TRUE(gpu_intersections[i].intersects);
            ASSERT_EQ(gpu_intersections[i].primitive, inter->index);
            ASSERT_TRUE(almost_equal(
                    gpu_intersections[i].distance, inter->inter.t, 10));
        }
    }
}
