#include <doctest/doctest.h>
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"

using namespace Spire;

TEST_SUITE("InteractionsKeyBindings") {
  TEST_CASE("global") {
    auto interactions = InteractionsKeyBindingsModel();
    REQUIRE(interactions.get_default_quantity()->get() == 100);
    REQUIRE(interactions.get_quantity_increment(Qt::NoModifier)->get() == 100);
    REQUIRE(
      interactions.get_quantity_increment(Qt::ShiftModifier)->get() == 100);
    REQUIRE(interactions.get_quantity_increment(Qt::AltModifier)->get() == 100);
    REQUIRE(
      interactions.get_quantity_increment(Qt::ControlModifier)->get() == 100);
    REQUIRE(!interactions.is_cancel_on_fill()->get());
  }

  TEST_CASE("parent") {
    auto parent = std::make_shared<InteractionsKeyBindingsModel>();
    parent->get_default_quantity()->set(123);
    parent->is_cancel_on_fill()->set(true);
    auto interactions = InteractionsKeyBindingsModel(parent);
    REQUIRE(interactions.get_default_quantity()->get() == 123);
    REQUIRE(interactions.is_cancel_on_fill()->get());
    parent->get_default_quantity()->set(400);
    REQUIRE(interactions.get_default_quantity()->get() == 400);
    interactions.is_cancel_on_fill()->set(false);
    REQUIRE(interactions.get_default_quantity()->get() == 400);
    REQUIRE(!interactions.is_cancel_on_fill()->get());
    REQUIRE(parent->is_cancel_on_fill()->get());
    parent->get_default_quantity()->set(514);
    REQUIRE(interactions.get_default_quantity()->get() == 400);
  }
}
