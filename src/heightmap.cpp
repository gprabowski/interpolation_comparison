#include <heightmap.hpp>

namespace pusn {
void heightmap::regenerate_mesh() {
  geometry.vertices.clear();
  geometry.indices.clear();
  // 1. Fill vertices
  math::vec3 normal = {0, 1, 0};
  auto half_x = mesh_size.x * 0.5f;
  auto half_y = mesh_size.y * 0.5f;
  for (int y = 0; y <= mesh_detail.y; ++y) {
    for (int x = 0; x <= mesh_detail.x; ++x) {
      float tex_x =
          std::clamp(static_cast<float>(x) / static_cast<float>(mesh_detail.x),
                     0.f, 1.f); // 0 to 1
      float tex_y =
          std::clamp(static_cast<float>(y) / static_cast<float>(mesh_detail.y),
                     0.f, 1.f); // 0 to 1

      math::vec3 pos{mesh_size.x * tex_x - half_x, 0,
                     mesh_size.y * tex_y - half_y};

      geometry.vertices.push_back({pos, normal, {tex_x, tex_y}});
    }
  }

  // 2. Fill indices
  for (int y = 0; y < mesh_detail.y; ++y) {
    for (int x = 0; x < mesh_detail.x; ++x) {
      geometry.indices.push_back(reindexer(x, y));
      geometry.indices.push_back(reindexer(x + 1, y));
      geometry.indices.push_back(reindexer(x, y + 1));
      geometry.indices.push_back(reindexer(x + 1, y + 1));
    }
  }

  glfw_impl::fill_renderable(geometry.vertices, geometry.indices,
                             api_renderable);
}
} // namespace pusn
