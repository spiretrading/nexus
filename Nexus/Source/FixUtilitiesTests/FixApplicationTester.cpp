#include <doctest/doctest.h>
#include <quickfix/SessionID.h>
#include <quickfix/SessionSettings.h>
#include "Nexus/FixUtilities/FixApplication.hpp"

using namespace Nexus;

namespace {
  struct TestFixApplication : FixApplication {
    std::shared_ptr<Order>
        recover(const SequencedAccountOrderRecord&) override {
      return nullptr;
    }

    std::shared_ptr<Order> submit(const OrderInfo&) override {
      return nullptr;
    }

    void cancel(const OrderExecutionSession&, OrderId) override {}

    void update(
      const OrderExecutionSession&, OrderId, const ExecutionReport&) override {}

    void onCreate(const FIX::SessionID&) override {}

    void onLogon(const FIX::SessionID&) override {}

    void onLogout(const FIX::SessionID&) override {}

    void toAdmin(FIX::Message&, const FIX::SessionID&) override {}

    void toApp(FIX::Message&, const FIX::SessionID&) override {}

    void fromAdmin(const FIX::Message&, const FIX::SessionID&) override {}

    void fromApp(const FIX::Message&, const FIX::SessionID&) override {}

    using FixApplication::set_session_settings;
  };
}

TEST_SUITE("fix_application") {
  TEST_CASE("session_settings_getters") {
    auto session_id = FIX::SessionID("FIX.4.2", "SENDER", "TARGET");
    auto settings_stream = std::stringstream();
    settings_stream <<
      "[SESSION]\n"
      "ConnectionType=initiator\n"
      "BeginString=FIX.4.2\n"
      "SenderCompID=SENDER\n"
      "TargetCompID=TARGET\n";
    auto session_settings = FIX::SessionSettings(settings_stream);
    auto application = TestFixApplication();
    application.set_session_settings(session_id, session_settings);
    REQUIRE(application.get_session_id() == session_id);
    REQUIRE(application.get_session_settings().get(
      session_id).getString("BeginString") == "FIX.4.2");
    REQUIRE(application.get_session_settings().get(
      session_id).getString("SenderCompID") == "SENDER");
    REQUIRE(application.get_session_settings().get(
      session_id).getString("TargetCompID") == "TARGET");
  }
}
