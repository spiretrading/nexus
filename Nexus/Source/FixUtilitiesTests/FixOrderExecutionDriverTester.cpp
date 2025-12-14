#include <doctest/doctest.h>
#include <quickfix/SessionID.h>
#include <quickfix/SessionSettings.h>
#include "Nexus/FixUtilities/FixOrderExecutionDriver.hpp"

using namespace Nexus;

namespace {
  struct DummyFixApplication : FixApplication {
    std::shared_ptr<Order> recover(
        const SequencedAccountOrderRecord&) override {
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
  };
}

TEST_SUITE("FixApplicationEntry") {
  TEST_CASE("destination_routing") {
    auto settings_stream = std::stringstream();
    settings_stream <<
      "[DEFAULT]\n"
      "ConnectionType=initiator\n"
      "StartTime=00:00:00\n"
      "EndTime=23:59:59\n"
      "heartbeat_interval=30\n"
      "FileStorePath=.\n"
      "SenderCompID=SENDER\n"
      "TargetCompID=TARGET\n"
      "[SESSION]\n"
      "BeginString=FIX.4.2\n"
      "SenderCompID=SENDER\n"
      "TargetCompID=TARGET\n";
    auto session_settings = FIX::SessionSettings(settings_stream);
    auto application = std::make_shared<DummyFixApplication>();
    auto destinations = std::vector<std::string>{"TSX", "TSXV", "ALPHA"};
    auto entry =
      FixApplicationEntry(session_settings, destinations, application);
  }
}
