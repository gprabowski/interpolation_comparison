#pragma once

#include <inputs.hpp>
#include <milling_scene.hpp>

namespace pusn {

namespace chosen_api = glfw_impl;

struct milling_simulator {
  // graphical API object
  chosen_api::window_t window;
  chosen_api::frambuffer viewport;

  // input state object
  input_state input;
  // scene object
  milling_scene scene;

  // functions
  // init all systems
  bool init(const std::string &window_title);
  bool main_loop();
  void process_input();
  void render_viewport();
  void render_gui();
};

} // namespace pusn
