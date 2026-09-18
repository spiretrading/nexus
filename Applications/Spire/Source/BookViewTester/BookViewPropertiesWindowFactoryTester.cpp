#include <doctest/doctest.h>
#include "Spire/BookView/BookViewPropertiesWindowFactory.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  const auto TICKER = parse_ticker("ABX.TSX");
  const auto OTHER_TICKER = parse_ticker("MSFT.TSX");
  const auto THIRD_TICKER = parse_ticker("BB.TSX");
}

TEST_SUITE("BookViewPropertiesWindowFactory") {
  TEST_CASE("reset_then_cancel_restores_custom_interactions") {
    run_test([] {
      auto key_bindings = std::make_shared<KeyBindingsModel>();
      auto interactions = key_bindings->get_interactions_key_bindings(TICKER);
      interactions->get_default_quantity()->set(500);
      auto factory = BookViewPropertiesWindowFactory();
      auto ticker = make_local_value_model(TICKER);
      auto preview = make_proxy_value_model(factory.get_properties());
      factory.make(key_bindings, ticker, preview);
      interactions->reset();
      REQUIRE(!interactions->is_detached());
      ticker->set(OTHER_TICKER);
      REQUIRE(interactions->is_detached());
      REQUIRE(interactions->get_default_quantity()->get() == 500);
    });
  }

  TEST_CASE("edit_then_cancel_reattaches_interactions") {
    run_test([] {
      auto key_bindings = std::make_shared<KeyBindingsModel>();
      auto interactions = key_bindings->get_interactions_key_bindings(TICKER);
      auto factory = BookViewPropertiesWindowFactory();
      auto ticker = make_local_value_model(TICKER);
      auto preview = make_proxy_value_model(factory.get_properties());
      factory.make(key_bindings, ticker, preview);
      interactions->get_default_quantity()->set(500);
      REQUIRE(interactions->is_detached());
      ticker->set(OTHER_TICKER);
      REQUIRE(!interactions->is_detached());
    });
  }

  TEST_CASE("editing_after_a_ticker_change_leaves_the_previous_ticker") {
    run_test([] {
      auto key_bindings = std::make_shared<KeyBindingsModel>();
      auto interactions = key_bindings->get_interactions_key_bindings(TICKER);
      auto factory = BookViewPropertiesWindowFactory();
      auto ticker = make_local_value_model(TICKER);
      auto preview = make_proxy_value_model(factory.get_properties());
      factory.make(key_bindings, ticker, preview);
      ticker->set(OTHER_TICKER);
      auto other = key_bindings->get_interactions_key_bindings(OTHER_TICKER);
      other->get_default_quantity()->set(300);
      REQUIRE(other->is_detached());
      REQUIRE(!interactions->is_detached());
    });
  }

  TEST_CASE("preview_follows_the_previewing_window") {
    run_test([] {
      auto key_bindings = std::make_shared<KeyBindingsModel>();
      auto factory = BookViewPropertiesWindowFactory();
      auto ticker = make_local_value_model(TICKER);
      auto other_ticker = make_local_value_model(OTHER_TICKER);
      auto preview = make_proxy_value_model(factory.get_properties());
      auto other_preview = make_proxy_value_model(factory.get_properties());
      factory.make(key_bindings, ticker, preview);
      factory.make(key_bindings, other_ticker, other_preview);
      auto other = key_bindings->get_interactions_key_bindings(OTHER_TICKER);
      other->get_default_quantity()->set(300);
      REQUIRE(other->is_detached());
      ticker->set(THIRD_TICKER);
      REQUIRE(other->is_detached());
      REQUIRE(other->get_default_quantity()->get() == 300);
    });
  }
}
