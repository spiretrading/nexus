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
    nodes.push_back(new QWidget(nodes[0]));
    nodes.push_back(new QWidget(nodes[0]));
    nodes.push_back(new QWidget(nodes[1]));
    nodes.push_back(new QWidget(nodes[1]));
    nodes.push_back(new QWidget(nodes[1]));
    nodes.push_back(new QWidget(nodes[2]));
    nodes.push_back(new QWidget(nodes[2]));
    nodes.push_back(new QWidget(nodes[2]));
    return nodes;
  }
}

TEST_SUITE("SiblingSelector") {
  TEST_CASE("equality") {
    REQUIRE(SiblingSelector(Any(), Any()) == SiblingSelector(Any(), Any()));
    REQUIRE(SiblingSelector(Any(), Any()) != SiblingSelector(Any(), Hover()));
    REQUIRE(SiblingSelector(Any(), Any()) != SiblingSelector(Hover(), Any()));
    REQUIRE(SiblingSelector(Any(), Hover()) != SiblingSelector(Any(), Any()));
    REQUIRE(SiblingSelector(Hover(), Any()) != SiblingSelector(Any(), Any()));
  }
}
