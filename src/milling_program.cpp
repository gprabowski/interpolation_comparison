#include <milling_program.hpp>
#include <utils.hpp>

namespace pusn {

parsing_result parse_lines_into_program(std::vector<std::string> &lines) {
  milling_program ret{};
  math::vec3 last_pos{0.f, 0.f, 0.f};
  for (auto &line : lines) {
    milling_instruction instruction;
    int n;
    float x, y, z;
    auto count = sscanf(line.c_str(), "N%dG01X%fY%fZ%f", &n, &x, &y, &z);
    if (count == 4) {
      instruction.type = instruction_type::movement;
      instruction.load.position = {x, z, y};
      ret.instructions.push_back(instruction);
      last_pos = instruction.load.position;
      continue;
    }

    count = sscanf(line.c_str(), "N%dG01Y%fZ%f", &n, &y, &z);
    if (count == 3) {
      instruction.type = instruction_type::movement;
      instruction.load.position = {last_pos.x, z, y};
      ret.instructions.push_back(instruction);
      last_pos = instruction.load.position;
      continue;
    }

    count = sscanf(line.c_str(), "N%dG01X%fZ%f", &n, &x, &z);
    if (count == 3) {
      instruction.type = instruction_type::movement;
      instruction.load.position = {x, z, last_pos.z};
      ret.instructions.push_back(instruction);
      last_pos = instruction.load.position;
      continue;
    }

    count = sscanf(line.c_str(), "N%dG01X%fY%f", &n, &x, &y);
    if (count == 3) {
      instruction.type = instruction_type::movement;
      instruction.load.position = {x, last_pos.y, y};
      ret.instructions.push_back(instruction);
      last_pos = instruction.load.position;
      continue;
    }

    count = sscanf(line.c_str(), "N%dG01Z%f", &n, &z);
    if (count == 2) {
      instruction.type = instruction_type::movement;
      instruction.load.position = {last_pos.x, z, last_pos.z};
      ret.instructions.push_back(instruction);
      last_pos = instruction.load.position;
      continue;
    }

    count = sscanf(line.c_str(), "N%dG01Y%f", &n, &y);
    if (count == 2) {
      instruction.type = instruction_type::movement;
      instruction.load.position = {last_pos.x, last_pos.y, y};
      ret.instructions.push_back(instruction);
      last_pos = instruction.load.position;
      continue;
    }

    count = sscanf(line.c_str(), "N%dG01X%f", &n, &x);
    if (count == 2) {
      instruction.type = instruction_type::movement;
      instruction.load.position = {x, last_pos.y, last_pos.z};
      ret.instructions.push_back(instruction);
      last_pos = instruction.load.position;
      continue;
    }

    return {"incorrect instruction"};
  }

  for (auto &instruction : ret.instructions) {
    instruction.load.position.z *= -1;
  }

  return {{}, ret};
}

parsing_result read_instructions(std::filesystem::path filepath) {
  milling_tool_type type;
  int radius;

  if (!std::filesystem::exists(filepath)) {
    return {"File doesn't exist"};
  }

  std::string ext = filepath.extension().string();
  if (ext[1] == 'k') {
    type = milling_tool_type::spherical;
  } else if (ext[1] == 'f') {
    type = milling_tool_type::flat;
  } else {
    return {"first extension letter must be either a k or an f"};
  }

  if (!isdigit(ext[2]) || (ext.length() >= 4 && !std::isdigit(ext[3]))) {
    return {"after tool type there must be a numeric radius in the extension"};
  } else {
    radius = ext[2] - '0';
    if (ext.length() >= 4) {
      radius = radius * 10 + ext[3] - '0';
    }

    radius = (radius == 1) ? 1 : radius / 2;
  }

  auto program_lines = utils::read_text_lines_file(filepath);

  auto parsed = parse_lines_into_program(program_lines);
  if (parsed.result_program.has_value()) {
    parsed.result_program.value().tool_type = type;
    parsed.result_program.value().tool_radius = radius;
  }

  return parsed;
}

void generate_path_geometry(milling_program &program) {
  program.paths_geometry.indices.clear();
  program.paths_geometry.vertices.clear();

  int idx = 0;
  for (auto &instruction : program.instructions) {
    if (instruction.type == instruction_type::movement) {
      program.paths_geometry.vertices.push_back(
          {instruction.load.position, {}, {}});
      program.paths_geometry.indices.push_back(idx++);
    }
  }
}

} // namespace pusn
