#include <stamping.hpp>

namespace pusn {

bool produces_correct_configuration(milling_tool_type type,
                                    float cutting_length, float h, float &out,
                                    const math::vec3 &a, const math::vec3 &b) {
  if (h < 0.f) {
    LOGGER_ERROR("Trying to mill negative heights");
    return false;
  }

  auto cuts = h < out;

  if (type == milling_tool_type::flat && b.y < a.y && cuts) {
    LOGGER_ERROR("Trying to move flat tool down");
    return false;
  }

  if (cuts && out - h > cutting_length) {
    LOGGER_ERROR(
        "You're breaking the milling tool by cutting with the wrong part");
    return false;
  }

  return true;
}

void fill_stamp(milling_tool_type type, int radius, internal::tool_stamp &stamp,
                int pixels_per_unit) {
  stamp.stamp_values.clear();
  const auto pixel_radius = radius * pixels_per_unit;

  stamp.width = 2 * pixel_radius + 1;
  stamp.height = 2 * pixel_radius + 1;

  if (type == milling_tool_type::flat) {
    stamp.stamp_values.resize(stamp.width * stamp.height, 0.f);
    for (int w = -pixel_radius; w <= pixel_radius; ++w) {
      for (int h = -pixel_radius; h <= pixel_radius; ++h) {
        double f_w = (static_cast<double>(w) + 0.5) / pixels_per_unit;
        double f_h = (static_cast<double>(h) + 0.5) / pixels_per_unit;
        if (f_w * f_w + f_h * f_h > radius * radius) {
          const auto tmp_w = w + pixel_radius;
          const auto tmp_h = h + pixel_radius;
          stamp.stamp_values[tmp_h * stamp.width + tmp_w] = -1.f;
        }
      }
    }
  } else if (type == milling_tool_type::spherical) {
    stamp.stamp_values.resize(stamp.width * stamp.height, -1.f);
    for (int w = -pixel_radius; w <= pixel_radius; ++w) {
      for (int h = -pixel_radius; h <= pixel_radius; ++h) {
        const auto tmp_w = w + pixel_radius;
        const auto tmp_h = h + pixel_radius;

        math::vec2 p{w, h};
        auto r = glm::length(p) / pixel_radius;
        r *= radius;
        if (p.x * p.x + p.y * p.y <= pixel_radius * pixel_radius) {
          auto h = std::sqrt(radius * radius - r * r);
          stamp.stamp_values[tmp_h * stamp.width + tmp_w] =
              (h < 0) ? h : radius - h;
        }
      }
    }
  }
}

void stamp_here(int x, int y, int tex_w, int tex_h, std::vector<float> &pixels,
                internal::milling_tool &tool, milling_tool_type &tool_type,
                const math::vec3 &a, const math::vec3 &b) {
#pragma omp parallel for num_threads(4)
  for (int h = -tool.stamp.height / 2; h <= tool.stamp.height / 2; ++h) {
    for (int w = -tool.stamp.width / 2; w <= tool.stamp.width / 2; ++w) {
      int tex_x = x + w;
      int tex_y = y + h;
      if (tex_x >= 0 && tex_x < tex_w && tex_y >= 0 && tex_y < tex_h) {
        auto idx = tex_y * tex_w + tex_x;
        const auto sv =
            tool.stamp
                .stamp_values[(h + tool.stamp.height / 2) * tool.stamp.width +
                              (w + tool.stamp.width / 2)];
        float val = (sv < 0) ? pixels[idx]
                             : ((tool.placement.position.y + sv > pixels[idx])
                                    ? pixels[idx]
                                    : tool.placement.position.y + sv);
        if (produces_correct_configuration(tool_type, tool.cutting_length,
                                           tool.placement.position.y + sv,
                                           pixels[idx], a, b)) {
          pixels[idx] = val;
        }
      }
    }
  }
}

} // namespace pusn
