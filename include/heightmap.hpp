#pragma once

#include <geometry.hpp>
#include <glfw_impl.hpp>

namespace pusn {

template <typename StampMethod>
void Bresenham(int xa, int ya, int xb, int yb, StampMethod putpixel) {
  if (abs(xa - xb) > abs(ya - yb)) {
    int dx = std::abs(xa - xb), dy = std::abs(ya - yb);
    float total = (dx + dy) + 1;
    int steps = 0;
    int p = 2 * dy - dx;
    int twoDy = 2 * dy, twoDyDx = 2 * (dy - dx);
    int x, y, xEnd;

    bool xnormal = xa < xb;
    bool ynormal = ya < yb;

    x = xa;
    y = ya;
    xEnd = xb;
    putpixel(x, y, steps / total);
    while (xnormal ? x < xEnd : x > xEnd) {
      xnormal ? x++ : x--;
      ++steps;
      if (p < 0) {
        p = p + twoDy;
      } else {
        ++steps;
        ynormal ? y++ : y--;
        p = p + twoDyDx;
      }

      putpixel(x, y, steps / total);
    }
  } else {
    int dx = std::abs(xa - xb), dy = std::abs(ya - yb);
    float total = (dx + dy) + 1;
    int steps = 0;
    int p = 2 * dx - dy;
    int twoDx = 2 * dx, twoDxDy = 2 * (dx - dy);
    int x, y, yEnd;

    bool xnormal = xa < xb;
    bool ynormal = ya < yb;

    x = xa;
    y = ya;
    yEnd = yb;

    putpixel(x, y, steps / total);
    while (ynormal ? y < yEnd : y > yEnd) {
      ynormal ? y++ : y--;
      ++steps;
      if (p < 0) {
        p = p + twoDx;
      } else {
        ++steps;
        xnormal ? x++ : x--;
        p = p + twoDxDy;
      }

      putpixel(x, y, steps / total);
    }
  }
}

struct heightmap_texture {
  int width{0}, height{0};
  std::vector<float> pixels;

  inline void resize(int x, int y) {
    width = x;
    height = y;
    pixels.resize(width * height);
  }

  inline void reset(float val) {
    pixels.clear();
    pixels.resize(width * height, val);
  }
};

struct heightmap {
  api_agnostic_geometry geometry{
      {{math::vec3(-1.0, 0.0, -1.0), {0, 1, 0}, {0.0, 0.0}},
       {math::vec3(1.0, 0.0, -1.0), {0, 1, 0}, {1.0, 0.0}},
       {math::vec3(-1.0, 0.0, 1.0), {0, 1, 0}, {0.0, 1.0}},
       {math::vec3(1.0, 0.0, 1.0), {0, 1, 0}, {1.0, 1.0}}},
      {0, 1, 2, 3, 2, 1}};
  scene_object_info placement{{0.f, 0.f, 0.f}, {}, {1.f, 1.f, 1.f}};
  glfw_impl::renderable api_renderable;

  math::int2 mesh_detail{10, 10};
  math::int2 mesh_size{150, 150};
  int pixels_per_unit{10};
  float depth = 50.f;

  glfw_impl::texture_t color_tex;

  heightmap_texture cpu_texture;
  glfw_impl::texture_t gpu_texture;

  void regenerate_mesh();
  inline int reindexer(int x, int y) { return y * (mesh_detail.x + 1) + x; };
};
}; // namespace pusn
