#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Version.hpp"

int main(int argc, const char** argv) {
  auto g = Nexus::TestEnvironment();
  auto r = Beam::Routines::RoutineHandlerGroup();
  for(auto i = 0UL; i < boost::thread::hardware_concurrency(); ++i) {
    r.Spawn([] {
      auto t = Nexus::TestEnvironment();
    });
  }
  return 0;
}
