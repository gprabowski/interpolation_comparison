#pragma once

#include <chrono>
#include <vector>

#include <glad/glad.h>

#include <geometry.hpp>
#include <glfw_impl.hpp>
#include <mock_data.hpp>

#include <atomic>

namespace pusn {

namespace internal {
// each of them needs to store:
//    * geometry
//    * API object reference

struct simulation_settings {
  float length{5.f};
  decltype(std::chrono::system_clock::now()) start_time;
  math::vec3 position_start{0.f, 0.f, 0.f};
  math::vec3 position_end{500.f, 0.f, 0.f};

  glm::quat quat_rotation_start{1.f, 0.f, 1.f, 1.f};
  glm::quat quat_rotation_end{1.f, 0.f, 1.f, 1.f};

  glm::vec3 euler_rotation_start{0.f, 0.f, 0.f};
  glm::vec3 euler_rotation_end{2 * glm::pi<float>(), 0.f, 0.f};

  bool slerp{true};
  bool animation{true};
  int frames{10};
};

struct light {
  scene_object_info placement{{200.f, 100.f, 200.f}, {}, {}};
  math::vec3 color{1.f, 1.f, 1.f};
};

struct scene_grid {
  api_agnostic_geometry geometry{{{math::vec3(-1.0, 0.0, -1.0), {}, {}},
                                  {math::vec3(1.0, 0.0, -1.0), {}, {}},
                                  {math::vec3(1.0, 0.0, 1.0), {}, {}},
                                  {math::vec3(-1.0, 0.0, 1.0), {}, {}}},
                                 {0, 1, 2, 2, 3, 0}};
  scene_object_info placement;
  glfw_impl::renderable api_renderable;
};

struct model {
  float height{70.f};
  float radius{5.f};

  std::optional<simulation_settings> current_settings;
  simulation_settings next_settings;

  std::vector<scene_object_info> left_placements{
      scene_object_info{{0.f, 100.f, 0.f}, {0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}}};

  std::vector<scene_object_info> right_placements{
      scene_object_info{{0.f, -100.f, 0.f}, {0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}}};

  api_agnostic_geometry geometry;
  glfw_impl::renderable api_renderable;

  inline void reset() {
    geometry.vertices.clear();
    geometry.indices.clear();
    mock_data::buildVerticesSmooth(100, height, radius, geometry.vertices,
                                   geometry.indices);
    glfw_impl::fill_renderable(geometry.vertices, geometry.indices,
                               api_renderable);
  }
};

} // namespace internal

struct interpolator_scene {
  internal::simulation_settings settings;
  internal::model model;
  internal::scene_grid grid;
  internal::light light;

  bool init();
  void render(input_state &input, bool left = true);
  void set_light_uniforms(input_state &input, glfw_impl::renderable &r);
};

} // namespace pusn
