#pragma once

#include <milling_scene.hpp>

namespace pusn {

void fill_stamp(milling_tool_type type, int radius, internal::tool_stamp &stamp,
                int pixels_per_unit);

void stamp_here(int x, int y, int tex_w, int tex_h, std::vector<float> &pixels,
                internal::milling_tool &tool, milling_tool_type &tool_type,
                const math::vec3 &a, const math::vec3 &b);

struct stamper {
  internal::simulation_settings &settings;
  milling_program &program;
  internal::milling_tool &tool;
  heightmap &model;
  // Check for errors
  void operator()() {
    settings.is_finished = false;
    for (std::size_t idx = 1; idx < program.paths_geometry.vertices.size();
         ++idx) {
      if (settings.should_exit)
        return;
      // 1. find positions in x and z
      int xa = program.paths_geometry.vertices[idx - 1].pos.x *
               model.pixels_per_unit;
      int za = program.paths_geometry.vertices[idx - 1].pos.z *
               model.pixels_per_unit;
      int xb =
          program.paths_geometry.vertices[idx].pos.x * model.pixels_per_unit;
      int zb =
          program.paths_geometry.vertices[idx].pos.z * model.pixels_per_unit;
      int half_texture_x = model.cpu_texture.width / 2;
      int half_texture_y = model.cpu_texture.height / 2;
      // 2.
      if (xa != xb || za != zb) {
        Bresenham(xa, za, xb, zb, [&](int x, int z, float progress) {
          tool.placement.position.x =
              static_cast<double>(x) / model.pixels_per_unit;
          tool.placement.position.y =
              (1 - progress) * program.paths_geometry.vertices[idx - 1].pos.y +
              progress * program.paths_geometry.vertices[idx].pos.y;
          tool.placement.position.z =
              static_cast<double>(z) / model.pixels_per_unit;
          if (settings.speed) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds{static_cast<int>(settings.speed)});
          }

          // STAMP ON HEIGHTMAP
          int center_tex_x = x + half_texture_x;
          int center_tex_z = z + half_texture_y;

          stamp_here(center_tex_x, center_tex_z, model.cpu_texture.width,
                     model.cpu_texture.height, model.cpu_texture.pixels, tool,
                     program.tool_type,
                     program.paths_geometry.vertices[idx - 1].pos,
                     program.paths_geometry.vertices[idx].pos);
        });
      } else {
        tool.placement.position = program.paths_geometry.vertices[idx].pos;
        int center_tex_x =
            tool.placement.position.x * model.pixels_per_unit + half_texture_x;
        int center_tex_z =
            tool.placement.position.z * model.pixels_per_unit + half_texture_y;
        stamp_here(center_tex_x, center_tex_z, model.cpu_texture.width,
                   model.cpu_texture.height, model.cpu_texture.pixels, tool,
                   program.tool_type,
                   program.paths_geometry.vertices[idx - 1].pos,
                   program.paths_geometry.vertices[idx].pos);
      }
    }

    settings.is_finished = true;
  }
};

} // namespace pusn
