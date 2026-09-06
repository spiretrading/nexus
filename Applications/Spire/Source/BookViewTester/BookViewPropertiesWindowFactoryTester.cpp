#include <doctest/doctest.h>
#include "Spire/BookView/BookViewPropertiesWindowFactory.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  const auto TICKER = parse_ticker("ABX.TSX");
  const auto OTHER_TICKER = parse_ticker("MSFT.TSX");
}

TEST_SUITE("BookViewPropertiesWindowFactory") {
  TEST_CASE("reset_then_cancel_restores_custom_interactions") {
    run_test([] {
      auto key_bindings = std::make_shared<KeyBindingsModel>();
      auto interactions = key_bindings->get_interactions_key_bindings(TICKER);
      interactions->get_default_quantity()->set(500);
      auto factory = BookViewPropertiesWindowFactory();
      auto preview = make_proxy_value_model(factory.get_properties());
      factory.make(key_bindings, TICKER, preview);
      interactions->reset();
      REQUIRE(!interactions->is_detached());
      factory.make(key_bindings, OTHER_TICKER, preview);
      REQUIRE(interactions->is_detached());
      REQUIRE(interactions->get_default_quantity()->get() == 500);
    });
  }

  TEST_CASE("edit_then_cancel_reattaches_interactions") {
    run_test([] {
      auto key_bindings = std::make_shared<KeyBindingsModel>();
      auto interactions = key_bindings->get_interactions_key_bindings(TICKER);
      auto factory = BookViewPropertiesWindowFactory();
      auto preview = make_proxy_value_model(factory.get_properties());
      factory.make(key_bindings, TICKER, preview);
      interactions->get_default_quantity()->set(500);
      REQUIRE(interactions->is_detached());
      factory.make(key_bindings, OTHER_TICKER, preview);
      REQUIRE(!interactions->is_detached());
    });
  }
}
