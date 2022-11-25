#include <milling_scene.hpp>

#include <vector>

#include <math.hpp>

#include <mock_data.hpp>

#include <stb_image/stb_image.h>

namespace pusn {

void generate_milling_tool(api_agnostic_geometry &out) {}

bool milling_scene::init() {
  // Generate and add milling tool
  mock_data::buildVerticesSmooth(100, tool.height, tool.radius,
                                 tool.geometry.vertices, tool.geometry.indices);
  glfw_impl::fill_renderable(tool.geometry.vertices, tool.geometry.indices,
                             tool.api_renderable);
  glfw_impl::add_program_to_renderable("resources/tool", tool.api_renderable);

  // add heightmap
  model.cpu_texture.resize(model.pixels_per_unit * model.mesh_size.x,
                           model.pixels_per_unit * model.mesh_size.y);

  model.cpu_texture.reset(model.depth);
  glfw_impl::fill_texture<float>(model.gpu_texture, model.cpu_texture.width,
                                 model.cpu_texture.height,
                                 model.cpu_texture.pixels.data());
  model.regenerate_mesh();
  glfw_impl::add_program_to_renderable("resources/heightmap",
                                       model.api_renderable);

  int width, height, nrChannels;
  unsigned char *data = stbi_load("resources/textures/granite.jpg", &width,
                                  &height, &nrChannels, 0);

  glfw_impl::fill_texture<unsigned char>(model.color_tex, width, height, data);

  // ADD GRID
  glfw_impl::fill_renderable(grid.geometry.vertices, grid.geometry.indices,
                             grid.api_renderable);
  glfw_impl::add_program_to_renderable("resources/grid", grid.api_renderable);

  return true;
}

void milling_scene::set_light_uniforms(input_state &input,
                                       glfw_impl::renderable &r) {
  // set light and camera uniforms
  glfw_impl::set_uniform("light_pos", r.program.value(),
                         light.placement.position);
  glfw_impl::set_uniform("light_color", r.program.value(), light.color);
  glfw_impl::set_uniform("cam_pos", r.program.value(), input.camera.pos);
}

void milling_scene::render(input_state &input) {
  // 1. get camera info
  glDepthFunc(GL_LESS);

  const auto view = math::get_view_matrix(
      input.camera.pos, input.camera.pos + input.camera.front, input.camera.up);

  const auto proj = math::get_projection_matrix(
      glm::radians(input.render_info.fov_y),
      glfw_impl::last_frame_info::viewport_area.x,
      glfw_impl::last_frame_info::viewport_area.y, input.render_info.clip_near,
      input.render_info.clip_far);

  glDisable(GL_CULL_FACE);
  const auto model_grid_m =
      math::get_model_matrix(grid.placement.position, grid.placement.scale,
                             math::deg_to_rad(grid.placement.rotation));
  glfw_impl::use_program(grid.api_renderable.program.value());

  set_light_uniforms(input, grid.api_renderable);

  glfw_impl::set_uniform("model", grid.api_renderable.program.value(),
                         model_grid_m);
  glfw_impl::set_uniform("view", grid.api_renderable.program.value(), view);
  glfw_impl::set_uniform("proj", grid.api_renderable.program.value(), proj);
  glfw_impl::render(grid.api_renderable, grid.geometry);

  // 4. render the model
  const auto model_heightmap_m =
      math::get_model_matrix(model.placement.position, model.placement.scale,
                             math::deg_to_rad(model.placement.rotation));
  glfw_impl::use_program(model.api_renderable.program.value());
  set_light_uniforms(input, model.api_renderable);
  glfw_impl::set_uniform("model", model.api_renderable.program.value(),
                         model_heightmap_m);
  glfw_impl::set_uniform("view", model.api_renderable.program.value(), view);
  glfw_impl::set_uniform("proj", model.api_renderable.program.value(), proj);

  glPatchParameteri(GL_PATCH_VERTICES, 4);
  glBindTextureUnit(1, model.gpu_texture.value());
  glUniform1i(
      glGetUniformLocation(model.api_renderable.program.value(), "height_map"),
      1);

  glBindTextureUnit(2, model.color_tex.value());
  glUniform1i(
      glGetUniformLocation(model.api_renderable.program.value(), "color"), 2);

  glfw_impl::render(model.api_renderable, model.geometry,
                    glfw_impl::render_mode::patches);
  glEnable(GL_CULL_FACE);

  // 5. render the tool
  const auto model_tool_m =
      math::get_model_matrix(tool.placement.position, tool.placement.scale,
                             math::deg_to_rad(tool.placement.rotation));
  glfw_impl::use_program(tool.api_renderable.program.value());
  set_light_uniforms(input, tool.api_renderable);
  glfw_impl::set_uniform("model", tool.api_renderable.program.value(),
                         model_tool_m);
  glfw_impl::set_uniform("view", tool.api_renderable.program.value(), view);
  glfw_impl::set_uniform("proj", tool.api_renderable.program.value(), proj);
  glfw_impl::render(tool.api_renderable, tool.geometry);

  // 6. render the paths
  glDisable(GL_DEPTH_TEST);
  if (program.has_value() && program.value().paths_visible) {
    const auto model_paths_m =
        math::get_model_matrix({0.f, 0.f, 0.f}, {1.f, 1.f, 1.f},
                               math::deg_to_rad(math::vec3{0.f, 0.f, 0.f}));
    auto shader_program_idx =
        program.value().paths_api_renderable.program.value();

    glfw_impl::use_program(shader_program_idx);
    set_light_uniforms(input, program.value().paths_api_renderable);
    glfw_impl::set_uniform("model", shader_program_idx, model_paths_m);
    glfw_impl::set_uniform("view", shader_program_idx, view);
    glfw_impl::set_uniform("proj", shader_program_idx, proj);
    glfw_impl::render(program.value().paths_api_renderable,
                      program.value().paths_geometry,
                      glfw_impl::render_mode::line_strip);
  }
  glEnable(GL_DEPTH_TEST);

  glfw_impl::use_program(0);
}
} // namespace pusn
