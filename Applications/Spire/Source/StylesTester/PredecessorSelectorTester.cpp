#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  std::vector<QWidget*> make_graph() {
    auto nodes = std::vector<QWidget*>();
    nodes.push_back(new QWidget());
    nodes.push_back(new QWidget(nodes.back()));
    nodes.push_back(new QWidget(nodes.back()));
    nodes.push_back(new QWidget(nodes.back()));
    nodes.push_back(new QWidget(nodes.back()));
    return nodes;
  }
}

TEST_SUITE("PredecessorSelector") {
  TEST_CASE("equality") {
    REQUIRE(
      PredecessorSelector(Any(), Any()) == PredecessorSelector(Any(), Any()));
    REQUIRE(
      PredecessorSelector(Any(), Any()) != PredecessorSelector(Any(), Hover()));
    REQUIRE(
      PredecessorSelector(Any(), Any()) != PredecessorSelector(Hover(), Any()));
    REQUIRE(
      PredecessorSelector(Any(), Hover()) != PredecessorSelector(Any(), Any()));
    REQUIRE(
      PredecessorSelector(Hover(), Any()) != PredecessorSelector(Any(), Any()));
  }
}
