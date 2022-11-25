#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <logger.hpp>
#include <string>

namespace pusn {
namespace utils {

std::string read_text_file(std::filesystem::path shader_file);

std::vector<std::string>
read_text_lines_file(const std::filesystem::path input_file);

} // namespace utils
} // namespace pusn
