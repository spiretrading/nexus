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

TEST_SUITE("SuccessorSelector") {
  TEST_CASE("equality") {
    REQUIRE(SuccessorSelector(Any(), Any()) == SuccessorSelector(Any(), Any()));
    REQUIRE(
      SuccessorSelector(Any(), Any()) != SuccessorSelector(Any(), Hover()));
    REQUIRE(
      SuccessorSelector(Any(), Any()) != SuccessorSelector(Hover(), Any()));
    REQUIRE(
      SuccessorSelector(Any(), Hover()) != SuccessorSelector(Any(), Any()));
    REQUIRE(
      SuccessorSelector(Hover(), Any()) != SuccessorSelector(Any(), Any()));
  }
}
