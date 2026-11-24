#include <atomic>
#include <Aspen/Aspen.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <doctest/doctest.h>
#include "Spire/Canvas/Tasks/Executor.hpp"

using namespace Beam;
using namespace Spire;

namespace {
  struct Fixture {
    Async<void> m_started;
    Async<void> m_release;
    Async<int> m_finished;
    std::weak_ptr<int> m_lifetime;
    std::unique_ptr<Executor> m_executor;

    Fixture()
        : m_executor(std::make_unique<Executor>()) {
      auto lifetime = std::make_shared<int>(123);
      m_lifetime = lifetime;
      m_executor->Add(Aspen::box(Aspen::lift([=, this] {
        m_started.get_eval().set();
        m_release.get();
        m_finished.get_eval().set(*lifetime);
      })));
      m_executor->Open();
      m_started.get();
    }
  };
}

TEST_SUITE("Executor") {
  TEST_CASE("shutdown_during_commit") {
    auto fixture = Fixture();
    auto is_destruction = false;
    SUBCASE("close") {}
    SUBCASE("destruction") {
      is_destruction = true;
    }
    auto is_returned = std::atomic_bool(false);
    auto shutdown = RoutineHandler(spawn([&] {
      if(is_destruction) {
        fixture.m_executor.reset();
      } else {
        fixture.m_executor->Close();
        fixture.m_executor->Close();
      }
      is_returned = true;
    }));
    flush_pending_routines();
    auto is_returned_before_commit = is_returned.load();
    auto is_retained = !fixture.m_lifetime.expired();
    fixture.m_release.get_eval().set();
    shutdown.wait();
    flush_pending_routines();
    fixture.m_executor.reset();
    REQUIRE(is_returned_before_commit);
    REQUIRE(is_retained);
    REQUIRE(fixture.m_finished.get() == 123);
    REQUIRE(fixture.m_lifetime.expired());
  }

  TEST_CASE("unopened") {
    auto lifetime = std::make_shared<int>(123);
    auto observer = std::weak_ptr(lifetime);
    {
      auto executor = Executor();
      executor.Add(Aspen::Box<void>(Aspen::lift([=] {
        return *lifetime;
      })));
      lifetime.reset();
      executor.Close();
    }
    REQUIRE(observer.expired());
  }
}
