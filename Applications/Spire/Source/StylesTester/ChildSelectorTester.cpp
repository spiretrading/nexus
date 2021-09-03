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

TEST_SUITE("ChildSelector") {
  TEST_CASE("equality") {
    REQUIRE(ChildSelector(Any(), Any()) == ChildSelector(Any(), Any()));
    REQUIRE(ChildSelector(Any(), Any()) != ChildSelector(Any(), Hover()));
    REQUIRE(ChildSelector(Any(), Any()) != ChildSelector(Hover(), Any()));
    REQUIRE(ChildSelector(Any(), Hover()) != ChildSelector(Any(), Any()));
    REQUIRE(ChildSelector(Hover(), Any()) != ChildSelector(Any(), Any()));
  }
}
