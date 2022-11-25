#include <milling_simulator.hpp>

int main() {
  pusn::milling_simulator sim;
  sim.init("Milling Simulator");
  sim.main_loop();
  return 0;
}
