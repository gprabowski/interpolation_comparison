#include <gui.hpp>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include <implot/implot.h>

#include <ImGuiFileDialog.h>

#include <milling_program.hpp>

#include <chrono>

#include <heightmap.hpp>

#include <stamping.hpp>

namespace pusn {
namespace gui {

struct gui_info {
  static std::string file_error_message;
};

std::string gui_info::file_error_message{""};

// utility structure for realtime plot
struct ScrollingBuffer {
  int MaxSize;
  int Offset;
  ImVector<ImVec2> Data;
  ScrollingBuffer(int max_size = 2000) {
    MaxSize = max_size;
    Offset = 0;
    Data.reserve(MaxSize);
  }
  void AddPoint(float x, float y) {
    if (Data.size() < MaxSize)
      Data.push_back(ImVec2(x, y));
    else {
      Data[Offset] = ImVec2(x, y);
      Offset = (Offset + 1) % MaxSize;
    }
  }
  void Erase() {
    if (Data.size() > 0) {
      Data.shrink(0);
      Offset = 0;
    }
  }
};

// utility structure for realtime plot
struct RollingBuffer {
  float Span;
  ImVector<ImVec2> Data;
  RollingBuffer() {
    Span = 10.0f;
    Data.reserve(2000);
  }
  void AddPoint(float x, float y) {
    float xmod = fmodf(x, Span);
    if (!Data.empty() && xmod < Data.back().x)
      Data.shrink(0);
    Data.push_back(ImVec2(xmod, y));
  }
};

void ShowDemo_RealtimePlots() {
  static ScrollingBuffer sdata1;
  static RollingBuffer rdata1;
  static float t = 0;
  t += ImGui::GetIO().DeltaTime;
  sdata1.AddPoint(t, chosen_api::last_frame_info::last_frame_time);
  rdata1.AddPoint(t, ImGui::GetIO().Framerate);

  static float history = 10.0f;
  ImGui::SliderFloat("History", &history, 1, 30, "%.1f s");
  rdata1.Span = history;

  static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;

  if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1, 150))) {
    ImPlot::SetupAxes(NULL, NULL, flags, flags);
    ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
    ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 16.f);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    ImPlot::PlotShaded("Frame time in ms", &sdata1.Data[0].x, &sdata1.Data[0].y,
                       sdata1.Data.size(), -INFINITY, 0, sdata1.Offset,
                       2 * sizeof(float));
    ImPlot::EndPlot();
  }
  if (ImPlot::BeginPlot("##Rolling", ImVec2(-1, 150))) {
    ImPlot::SetupAxes(NULL, NULL, flags, flags);
    ImPlot::SetupAxisLimits(ImAxis_X1, 0, history, ImGuiCond_Always);
    ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 100);
    ImPlot::PlotLine("FPS", &rdata1.Data[0].x, &rdata1.Data[0].y,
                     rdata1.Data.size(), 0, 0, 2 * sizeof(float));
    ImPlot::EndPlot();
  }
}

// color theme copied from thecherno/hazel
void set_dark_theme() {
  auto &colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

  // Headers
  colors[ImGuiCol_Header] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
  colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
  colors[ImGuiCol_HeaderActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  // Buttons
  colors[ImGuiCol_Button] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
  colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
  colors[ImGuiCol_ButtonActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  // Frame BG
  colors[ImGuiCol_FrameBg] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
  colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
  colors[ImGuiCol_FrameBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  // Tabs
  colors[ImGuiCol_Tab] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TabHovered] = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
  colors[ImGuiCol_TabActive] = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
  colors[ImGuiCol_TabUnfocused] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

  // Title
  colors[ImGuiCol_TitleBg] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TitleBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
}

bool init(chosen_api::window_t &w) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  // io.ConfigViewportsNoAutoMerge = true;
  // io.ConfigViewportsNoTaskBarIcon = true;

  ImGui::StyleColorsDark();

  // when viewports are enables we tweak WindowRounding/WIndowBg so platform
  // windows can look identical to regular ones
  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(w.get(), true);
  ImGui_ImplOpenGL3_Init("#version 460");

  // fonts
  io.FontDefault = io.Fonts->AddFontFromFileTTF(
      //"fonts/opensans/static/OpenSans/OpenSans-Regular.ttf",
      "resources/fonts/jbmono/fonts/ttf/JetBrainsMono-Regular.ttf", 18.0f);
  set_dark_theme();

  return true;
}

void start_frame() {
  static bool show_demo = false;
  ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockspace_flags);

  if (show_demo) {
    ImGui::ShowDemoWindow(&show_demo);
    ImPlot::ShowDemoWindow();
  }
}

void update_viewport_info(std::function<void(void)> process_input) {
  // update viewport static info
  ImGui::Begin("Viewport");

  const auto min = ImGui::GetWindowContentRegionMin();
  const auto max = ImGui::GetWindowContentRegionMax();

  chosen_api::last_frame_info::viewport_area = {max.x - min.x, max.y - min.y};

  auto tmp = ImGui::GetWindowPos();
  chosen_api::last_frame_info::viewport_pos = {tmp.x, tmp.y};
  chosen_api::last_frame_info::viewport_pos = {tmp.x + min.x, tmp.y + min.y};

  const ImVec2 cp = {chosen_api::last_frame_info::viewport_pos.x,
                     chosen_api::last_frame_info::viewport_pos.y};

  const ImVec2 ca = {chosen_api::last_frame_info::viewport_area.x,
                     chosen_api::last_frame_info::viewport_area.y};

  if (ImGui::IsMouseHoveringRect(cp, {cp.x + ca.x, cp.y + ca.y})) {
    process_input();
  }

  ImGui::End();
}

void render_performance_window() {
  ImGui::Begin("Frame Statistics");
  ShowDemo_RealtimePlots();
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::Text("Last CPU frame %.3lf ms",
              glfw_impl::last_frame_info::last_frame_time);
  ImGui::End();
}

void render_light_gui(internal::light &light) {
  ImGui::Begin("Light Settings");
  ImGui::DragFloat3("Position", glm::value_ptr(light.placement.position),
                    -10000.f, 10000.f);
  ImGui::ColorEdit3("Color", glm::value_ptr(light.color));
  ImGui::End();
}

void render_tool_gui(heightmap &model, internal::milling_tool &tool,
                     std::optional<milling_program> &program) {
  ImGui::Begin("Tool Settings");
  ImGui::DragFloat3("Position", glm::value_ptr(tool.placement.position), -100.f,
                    100.f);
  if (ImGui::DragFloat("Height", &tool.height, 1.f, 100.f)) {
    tool.reset();
  }

  if (ImGui::DragFloat("Cutting length", &tool.cutting_length, 1.f, 100.f)) {
    tool.reset();
  }

  if (ImGui::DragFloat("Radius", &tool.radius, 1.f, 100.f)) {
    tool.reset();
  }

  int tmp_x =
      static_cast<int>(tool.placement.position.x) * model.pixels_per_unit +
      model.cpu_texture.width / 2;
  int tmp_y =
      static_cast<int>(tool.placement.position.z) * model.pixels_per_unit +
      model.cpu_texture.height / 2;
  ImGui::Text("Texture position:  \n X: %d Y: %d\n", tmp_x, tmp_y);

  if (tool.stamp.stamp_values.size()) {
    if (ImGui::Button("Stamp Here")) {
      stamp_here(tmp_x, tmp_y, model.cpu_texture.width,
                 model.cpu_texture.height, model.cpu_texture.pixels, tool,
                 program.value().tool_type, {0, 0, 0}, {0, 1, 0});

      glfw_impl::fill_texture<float>(model.gpu_texture, model.cpu_texture.width,
                                     model.cpu_texture.height,
                                     model.cpu_texture.pixels.data());
    }
  }
  ImGui::End();
}

void render_model_gui(heightmap &model) {
  ImGui::Begin("Model settings");
  ImGui::DragFloat3("Position", glm::value_ptr(model.placement.position),
                    -10000.f, 10000.f);

  int tmp_size[2]{model.mesh_size.x, model.mesh_size.y};
  if (ImGui::DragInt2("Model size", tmp_size, 1, 1000)) {
    model.mesh_size = {tmp_size[0], tmp_size[1]};
    model.regenerate_mesh();

    model.cpu_texture.resize(model.pixels_per_unit * model.mesh_size.x,
                             model.pixels_per_unit * model.mesh_size.y);

    model.cpu_texture.reset(model.depth);
    glfw_impl::fill_texture<float>(model.gpu_texture, model.cpu_texture.width,
                                   model.cpu_texture.height,
                                   model.cpu_texture.pixels.data());
  }

  int tmp_detail[2]{model.mesh_detail.x, model.mesh_detail.y};
  if (ImGui::DragInt2("Mesh detail", tmp_detail, 1, 10)) {
    model.mesh_detail = {tmp_detail[0], tmp_detail[1]};
    model.regenerate_mesh();
  }

  if (ImGui::DragInt("Texels per unit", &model.pixels_per_unit, 5, 30)) {
    if (model.pixels_per_unit < 1)
      model.pixels_per_unit = 1;

    model.cpu_texture.resize(model.pixels_per_unit * model.mesh_size.x,
                             model.pixels_per_unit * model.mesh_size.y);

    model.cpu_texture.reset(model.depth);
    glfw_impl::fill_texture<float>(model.gpu_texture, model.cpu_texture.width,
                                   model.cpu_texture.height,
                                   model.cpu_texture.pixels.data());
  }

  if (ImGui::DragFloat("Depth", &model.depth, 1.f, 100.f)) {
    model.cpu_texture.reset(model.depth);
    glfw_impl::fill_texture<float>(model.gpu_texture, model.cpu_texture.width,
                                   model.cpu_texture.height,
                                   model.cpu_texture.pixels.data());
  }

  ImGui::End();
}

void render_program_gui(internal::simulation_settings &settings,
                        milling_program &program, internal::milling_tool &tool,
                        heightmap &model) {
  static std::string types[2]{"spherical", "flat"};

  ImGui::Begin("Milling program");
  ImGui::Text("Instruction Count: %d", program.instruction_count);
  ImGui::Text("Tool Radius : %d", program.tool_radius);
  ImGui::Text("Tool Type: %s",
              types[program.tool_type == milling_tool_type::flat].c_str());
  ImGui::Checkbox("Show paths", &program.paths_visible);

  ImGui::DragFloat("Simulation Speed", &settings.speed);

  if (ImGui::Button("Start")) {
    if (settings.background_worker.has_value()) {
      settings.should_exit = true;
      settings.background_worker.value().join();
      settings.background_worker.reset();
    }

    settings.quick_run = false;
    fill_stamp(program.tool_type, program.tool_radius, tool.stamp,
               model.pixels_per_unit);

    settings.should_exit = false;
    // 1. make sure that the tool starts at the right position
    if (program.paths_geometry.vertices.size()) {
      tool.placement.position = program.paths_geometry.vertices[0].pos;
    }
    // 2. run for all others
    settings.background_worker =
        std::thread(stamper{settings, program, tool, model});
  }

  if (ImGui::Button("Quick Milling")) {
    if (settings.background_worker.has_value()) {
      settings.should_exit = true;
      settings.background_worker.value().join();
      settings.background_worker.reset();
    }

    fill_stamp(program.tool_type, program.tool_radius, tool.stamp,
               model.pixels_per_unit);

    settings.should_exit = false;
    settings.quick_run = true;
    // 1. make sure that the tool starts at the right position
    if (program.paths_geometry.vertices.size()) {
      tool.placement.position = program.paths_geometry.vertices[0].pos;
    }
    // 2. run for all others
    settings.background_worker =
        std::thread(stamper{settings, program, tool, model});
  }

  if (ImGui::Button("Stop")) {
    settings.should_exit = true;
    if (program.instructions.size()) {
      tool.placement.position = program.paths_geometry.vertices[0].pos;
    }
  }

  if (settings.background_worker.has_value() && settings.is_finished) {
    settings.background_worker->join();
    settings.background_worker.reset();
    settings.is_finished = false;

    glfw_impl::fill_texture<float>(model.gpu_texture, model.cpu_texture.width,
                                   model.cpu_texture.height,
                                   model.cpu_texture.pixels.data());
  } else if (!settings.quick_run && settings.background_worker.has_value()) {
    static int counter = 0;
    if (++counter >= 60) {
      counter = 0;
      glfw_impl::fill_texture<float>(model.gpu_texture, model.cpu_texture.width,
                                     model.cpu_texture.height,
                                     model.cpu_texture.pixels.data());
    }
  }

  ImGui::End();
}

void render(input_state &input, milling_scene &scene) {
  render_performance_window();
  render_light_gui(scene.light);
  render_tool_gui(scene.model, scene.tool, scene.program);
  render_model_gui(scene.model);
  render_popups();
  render_main_menu(scene);
  if (scene.program.has_value()) {
    render_program_gui(scene.settings, scene.program.value(), scene.tool,
                       scene.model);
  }
}

void end_frame() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // update and render additional platform windows
  // (platform functions may change the current opengl context so we
  // save/restore it to make it easier to paste this code elsewhere. For
  // this specific demo appp we could also call
  // glfwMakeCOntextCurrent(window) directly)
  ImGuiIO &io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow *backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
}

static void show_menu_file(milling_scene &scene) {
  if (ImGui::MenuItem("Reset")) {
    scene.model.cpu_texture.reset(0);
    glfw_impl::fill_texture<float>(
        scene.model.gpu_texture, scene.model.cpu_texture.width,
        scene.model.cpu_texture.height, scene.model.cpu_texture.pixels.data());
  }

  if (ImGui::MenuItem("Open", "Ctrl+O")) {
    ImGuiFileDialog::Instance()->OpenDialog("OpenProgramChoice",
                                            "Choose Program File", ".*", ".*");
  }
}

void render_main_menu(milling_scene &scene) {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      show_menu_file(scene);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  if (ImGuiFileDialog::Instance()->Display("OpenProgramChoice")) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      std::string filepath = ImGuiFileDialog::Instance()->GetFilePathName();
      // action
      auto parsing_result = read_instructions(filepath);
      if (parsing_result.error_message.has_value()) {
        gui_info::file_error_message = parsing_result.error_message.value();
        ImGui::OpenPopup("File Corrupted");
      } else if (parsing_result.result_program.has_value()) {
        if (scene.program.has_value() &&
            scene.program.value().paths_api_renderable.program.has_value()) {
          auto tmp_vao = scene.program.value().paths_api_renderable.vao.value();
          auto tmp_vbo = scene.program.value().paths_api_renderable.vbo.value();
          auto tmp_ebo = scene.program.value().paths_api_renderable.ebo.value();
          GLuint tmp_program =
              scene.program.value().paths_api_renderable.program.value();
          scene.program.reset();

          glDeleteProgram(tmp_program);
          glDeleteBuffers(1, &tmp_vbo);
          glDeleteBuffers(1, &tmp_ebo);
          glDeleteVertexArrays(1, &tmp_vao);
        }

        scene.program = parsing_result.result_program.value();

        generate_path_geometry(scene.program.value());

        glfw_impl::add_program_to_renderable(
            "resources/paths", scene.program.value().paths_api_renderable);

        glfw_impl::fill_renderable(
            scene.program.value().paths_geometry.vertices,
            scene.program.value().paths_geometry.indices,
            scene.program.value().paths_api_renderable);

        scene.tool.radius = scene.program->tool_radius;
        if (scene.program.value().instructions.size()) {
          scene.tool.placement.position =
              scene.program.value().paths_geometry.vertices[0].pos;
        }
        scene.tool.reset();
        fill_stamp(scene.program.value().tool_type,
                   scene.program.value().tool_radius, scene.tool.stamp,
                   scene.model.pixels_per_unit);
      }
    }
    ImGuiFileDialog::Instance()->Close();
  }
}

void render_popups() {
  // Always center this window when appearing
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal("File Corrupted", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("The file you have pointed to is corrupted or wrongly "
                "formatted!\n\n");
    ImGui::Text("%s\n", gui_info::file_error_message.c_str());
    ImGui::Separator();

    if (ImGui::Button("OK", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

} // namespace gui
} // namespace pusn
