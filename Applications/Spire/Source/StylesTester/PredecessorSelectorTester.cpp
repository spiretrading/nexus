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

  TEST_CASE("selection") {
    run_test([] {
      auto selector = PredecessorSelector(Hover(), Focus());
      {
        auto graph = make_graph();
        match(*graph[4], Hover());
        match(*graph[3], Focus());
        {
          auto selection = select(selector, {&find_stylist(*graph[4])});
          REQUIRE(selection.size() == 1);
          REQUIRE(selection.contains(&find_stylist(*graph[3])));
        }
      }
      {
        auto graph = make_graph();
        match(*graph[4], Hover());
        match(*graph[2], Focus());
        {
          auto selection = select(selector, {&find_stylist(*graph[4])});
          REQUIRE(selection.size() == 1);
          REQUIRE(selection.contains(&find_stylist(*graph[2])));
        }
      }
      {
        auto graph = make_graph();
        match(*graph[4], Hover());
        match(*graph[1], Focus());
        {
          auto selection = select(selector, {&find_stylist(*graph[4])});
          REQUIRE(selection.size() == 1);
          REQUIRE(selection.contains(&find_stylist(*graph[1])));
        }
      }
      {
        auto graph = make_graph();
        match(*graph[4], Hover());
        match(*graph[0], Focus());
        {
          auto selection = select(selector, {&find_stylist(*graph[4])});
          REQUIRE(selection.size() == 1);
          REQUIRE(selection.contains(&find_stylist(*graph[0])));
        }
      }
      {
        auto graph = make_graph();
        match(*graph[4], Hover());
        match(*graph[3], Focus());
        match(*graph[2], Focus());
        {
          auto selection = select(selector, {&find_stylist(*graph[4])});
          REQUIRE(selection.size() == 1);
          REQUIRE(selection.contains(&find_stylist(*graph[3])));
        }
      }
    });
  }
}
