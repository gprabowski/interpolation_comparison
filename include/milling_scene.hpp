#pragma once

#include <vector>

#include <glad/glad.h>

#include <geometry.hpp>
#include <glfw_impl.hpp>
#include <heightmap.hpp>
#include <mock_data.hpp>

#include <milling_program.hpp>

#include <atomic>

namespace pusn {

namespace internal {
// each of them needs to store:
//    * geometry
//    * API object reference

struct simulation_settings {
  float speed{0.f};
  std::optional<std::thread> background_worker;
  std::atomic<bool> should_exit{false};
  std::atomic<bool> is_finished{false};
  std::atomic<bool> quick_run{false};
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

struct tool_stamp {
  int width{1}, height{1};
  std::vector<float> stamp_values;
};

struct milling_tool {
  float height{700.f};
  float radius{16.f};

  float cutting_length{radius * 2};

  scene_object_info placement{
      {0.f, 100.f, 0.f}, {90.f, 0.f, 0.f}, {1.f, 1.f, 1.f}};
  api_agnostic_geometry geometry;
  glfw_impl::renderable api_renderable;

  tool_stamp stamp;

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

struct milling_scene {
  heightmap model;
  internal::simulation_settings settings;
  internal::milling_tool tool;
  internal::scene_grid grid;
  internal::light light;
  std::optional<milling_program> program;

  bool init();
  void render(input_state &input);
  void set_light_uniforms(input_state &input, glfw_impl::renderable &r);
};

} // namespace pusn
