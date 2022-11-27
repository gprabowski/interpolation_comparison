#include <interpolator_scene.hpp>

#include <vector>

#include <math.hpp>

#include <mock_data.hpp>

namespace pusn {

void generate_milling_tool(api_agnostic_geometry &out) {}

bool interpolator_scene::init() {
  // Generate and add milling tool
  mock_data::buildVerticesSmooth(100, model.height, model.radius,
                                 model.geometry.vertices,
                                 model.geometry.indices);
  glfw_impl::fill_renderable(model.geometry.vertices, model.geometry.indices,
                             model.api_renderable);
  glfw_impl::add_program_to_renderable("resources/model", model.api_renderable);

  // ADD GRID
  glfw_impl::fill_renderable(grid.geometry.vertices, grid.geometry.indices,
                             grid.api_renderable);
  glfw_impl::add_program_to_renderable("resources/grid", grid.api_renderable);

  return true;
}

void interpolator_scene::set_light_uniforms(input_state &input,
                                            glfw_impl::renderable &r) {
  // set light and camera uniforms
  glfw_impl::set_uniform("light_pos", r.program.value(),
                         light.placement.position);
  glfw_impl::set_uniform("light_color", r.program.value(), light.color);
  glfw_impl::set_uniform("cam_pos", r.program.value(), input.camera.pos);
}

void interpolator_scene::render(input_state &input, bool left) {

  // 1. get camera info
  glDepthFunc(GL_LESS);

  const auto view = math::get_view_matrix(
      input.camera.pos, input.camera.pos + input.camera.front, input.camera.up);

  const auto proj = math::get_projection_matrix(
      glm::radians(input.render_info.fov_y),
      left ? glfw_impl::last_frame_info::left_viewport_area.x
           : glfw_impl::last_frame_info::right_viewport_area.x,
      left ? glfw_impl::last_frame_info::left_viewport_area.y
           : glfw_impl::last_frame_info::right_viewport_area.y,
      input.render_info.clip_near, input.render_info.clip_far);

  // 2. render grid
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
  glEnable(GL_CULL_FACE);

  // 3. render the model
  const auto time = std::chrono::system_clock::now();
  if (model.current_settings.has_value()) {

    std::chrono::duration<float> elapsed_seconds =
        time - model.current_settings.value().start_time;
    const float progress =
        elapsed_seconds.count() / model.current_settings.value().length;

    if (progress > 1.0) {
      model.current_settings.reset();
    } else {
      model.left_placements.clear();
      model.right_placements.clear();
      // left (quaternion)
      scene_object_info curr;
      curr.position =
          (1 - progress) * model.current_settings.value().position_start +
          progress * model.current_settings.value().position_end;

      if (model.current_settings.value().slerp) {
        curr.rotation =
            glm::degrees(glm::eulerAngles(glm::normalize(math::slerp(
                model.current_settings.value().quat_rotation_start,
                model.current_settings.value().quat_rotation_end, progress))));
      } else {
        curr.rotation = glm::degrees(glm::eulerAngles(glm::normalize(math::lerp(
            model.current_settings.value().quat_rotation_start,
            model.current_settings.value().quat_rotation_end, progress))));
      }

      model.left_placements.push_back(curr);

      // right (euler angles)
      // 1. check if reversed will be needed
      auto eu_st = model.current_settings.value().euler_rotation_start;
      auto eu_en = model.current_settings.value().euler_rotation_end;

      const float pi = glm::pi<float>();
      const float tau = 2 * glm::pi<float>();

      eu_st = {std::fmod(eu_st.x, tau), std::fmod(eu_st.y, tau),
               std::fmod(eu_st.z, tau)};

      eu_en = {std::fmod(eu_en.x, tau), std::fmod(eu_en.y, tau),
               std::fmod(eu_en.z, tau)};

      const auto dist = eu_en - eu_st;

      if (std::abs(dist.x) > pi) {
        if (eu_en.x > eu_st.x) {
          eu_st.x += tau;
        } else {
          eu_en.x += tau;
        }
      }
      if (std::abs(dist.y) > pi) {
        if (eu_en.y > eu_st.y) {
          eu_st.y += tau;
        } else {
          eu_en.y += tau;
        }
      }
      if (std::abs(dist.z) > pi) {
        if (eu_en.z > eu_st.z) {
          eu_st.z += tau;
        } else {
          eu_en.z += tau;
        }
      }

      curr.rotation = glm::degrees(glm::mix(eu_st, eu_en, progress));
      model.right_placements.push_back(curr);
    }
  }

  if (left) {
    for (auto &placement : model.left_placements) {
      const auto model_model_m =
          math::get_model_matrix(placement.position, placement.scale,
                                 math::deg_to_rad(placement.rotation));
      glfw_impl::use_program(model.api_renderable.program.value());
      set_light_uniforms(input, model.api_renderable);
      glfw_impl::set_uniform("model", model.api_renderable.program.value(),
                             model_model_m);
      glfw_impl::set_uniform("view", model.api_renderable.program.value(),
                             view);
      glfw_impl::set_uniform("proj", model.api_renderable.program.value(),
                             proj);
      glfw_impl::render(model.api_renderable, model.geometry);

      glfw_impl::use_program(0);
    }
  } else {
    for (auto &placement : model.right_placements) {
      const auto model_model_m =
          math::get_model_matrix(placement.position, placement.scale,
                                 math::deg_to_rad(placement.rotation));
      glfw_impl::use_program(model.api_renderable.program.value());
      set_light_uniforms(input, model.api_renderable);
      glfw_impl::set_uniform("model", model.api_renderable.program.value(),
                             model_model_m);
      glfw_impl::set_uniform("view", model.api_renderable.program.value(),
                             view);
      glfw_impl::set_uniform("proj", model.api_renderable.program.value(),
                             proj);
      glfw_impl::render(model.api_renderable, model.geometry);

      glfw_impl::use_program(0);
    }
  }
}
} // namespace pusn
