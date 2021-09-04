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

TEST_SUITE("ParentSelector") {
  TEST_CASE("equality") {
    REQUIRE(ParentSelector(Any(), Any()) == ParentSelector(Any(), Any()));
    REQUIRE(ParentSelector(Any(), Any()) != ParentSelector(Any(), Hover()));
    REQUIRE(ParentSelector(Any(), Any()) != ParentSelector(Hover(), Any()));
    REQUIRE(ParentSelector(Any(), Hover()) != ParentSelector(Any(), Any()));
    REQUIRE(ParentSelector(Hover(), Any()) != ParentSelector(Any(), Any()));
  }
}
