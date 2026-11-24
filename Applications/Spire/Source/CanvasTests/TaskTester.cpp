#include <atomic>
#include <Beam/Queues/CallbackQueueWriter.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Canvas/Tasks/Task.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

TEST_SUITE("Task") {
  UI_TEST_CASE("destruction_during_commit") {
    auto environment = TestEnvironment();
    auto profile = UserProfile("", false, false, {}, {},
      get_default_additional_tag_database(), {}, {}, {},
      Clients(std::in_place_type<TestClients>, Ref(environment)));
    auto started = Async<void>();
    auto release = Async<void>();
    auto is_closed = std::atomic_bool(false);
    auto task =
      std::make_unique<Task>(IntegerNode(123), DirectoryEntry(), Ref(profile));
    task->GetPublisher().monitor(callback<Task::StateEntry>(
      [&] (const auto& entry) {
        if(entry.m_state == Task::State::INITIALIZING) {
          started.get_eval().set();
          release.get();
        }
      },
      [&] (const auto&) {
        is_closed = true;
      }));
    task->Execute();
    started.get();
    task.reset();
    auto is_closed_before_commit = is_closed.load();
    release.get_eval().set();
    flush_pending_routines();
    REQUIRE_FALSE(is_closed_before_commit);
    REQUIRE(is_closed);
  }
}
