#pragma once

#include <filesystem>
#include <math.hpp>
#include <optional>
#include <vector>

#include <geometry.hpp>
#include <glfw_impl/common.hpp>

namespace pusn {

enum class instruction_type { movement };

enum class milling_tool_type { flat, spherical };

struct instruction_load {
  math::vec3 position;
};

struct milling_instruction {
  int index;
  instruction_type type;
  instruction_load load;
};

struct milling_program {
  milling_tool_type tool_type;
  int tool_radius;
  int instruction_count;
  std::vector<milling_instruction> instructions;

  api_agnostic_geometry paths_geometry;
  glfw_impl::renderable paths_api_renderable;
  bool paths_visible{false};
};

struct parsing_result {
  std::optional<std::string> error_message;
  std::optional<milling_program> result_program;
};

parsing_result read_instructions(std::filesystem::path filepath);

void generate_path_geometry(milling_program &program);

} // namespace pusn
