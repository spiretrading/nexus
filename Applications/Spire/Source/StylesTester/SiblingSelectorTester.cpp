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

  TEST_CASE("selection") {
    run_test([] {
      auto selector = SiblingSelector(Hover(), Focus());
      {
        auto graph = make_graph();
        for(auto node : graph) {
          match(*node, Hover());
          match(*node, Focus());
        }
        auto selection = select(selector, {&find_stylist(*graph[0])});
        REQUIRE(selection.empty());
        delete graph.front();
      }
      {
        auto graph = make_graph();
        for(auto node : graph) {
          match(*node, Hover());
          match(*node, Focus());
        }
        auto selection = select(selector, {&find_stylist(*graph[1])});
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(*graph[2])));
        delete graph.front();
      }
      {
        auto graph = make_graph();
        for(auto node : graph) {
          match(*node, Hover());
          match(*node, Focus());
        }
        auto selection = select(selector, {&find_stylist(*graph[2])});
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(*graph[1])));
        delete graph.front();
      }
      {
        auto graph = make_graph();
        match(*graph[3], Hover());
        auto selection = select(selector, {&find_stylist(*graph[3])});
        REQUIRE(selection.empty());
        delete graph.front();
      }
      {
        auto graph = make_graph();
        match(*graph[3], Hover());
        match(*graph[4], Focus());
        auto selection = select(selector, {&find_stylist(*graph[3])});
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(*graph[4])));
        delete graph.front();
      }
      {
        auto graph = make_graph();
        match(*graph[3], Hover());
        match(*graph[5], Focus());
        auto selection = select(selector, {&find_stylist(*graph[3])});
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(*graph[5])));
        delete graph.front();
      }
      {
        auto graph = make_graph();
        match(*graph[4], Focus());
        match(*graph[5], Focus());
        auto selection = select(selector, {&find_stylist(*graph[3])});
        REQUIRE(selection.empty());
        delete graph.front();
      }
      {
        auto graph = make_graph();
        match(*graph[3], Hover());
        match(*graph[4], Focus());
        match(*graph[5], Focus());
        auto selection = select(selector, {&find_stylist(*graph[3])});
        REQUIRE(selection.size() == 2);
        REQUIRE(selection.contains(&find_stylist(*graph[4])));
        REQUIRE(selection.contains(&find_stylist(*graph[5])));
        delete graph.front();
      }
      {
        auto graph = make_graph();
        match(*graph[4], Hover());
        auto selection = select(selector, {&find_stylist(*graph[4])});
        REQUIRE(selection.empty());
        delete graph.front();
      }
      {
        auto graph = make_graph();
        match(*graph[4], Hover());
        match(*graph[5], Focus());
        auto selection = select(selector, {&find_stylist(*graph[4])});
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(*graph[5])));
        delete graph.front();
      }
      {
        auto graph = make_graph();
        match(*graph[4], Hover());
        match(*graph[3], Focus());
        auto selection = select(selector, {&find_stylist(*graph[4])});
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(*graph[3])));
        delete graph.front();
      }
      {
        auto graph = make_graph();
        match(*graph[3], Focus());
        match(*graph[5], Focus());
        auto selection = select(selector, {&find_stylist(*graph[4])});
        REQUIRE(selection.empty());
        delete graph.front();
      }
      {
        auto graph = make_graph();
        match(*graph[3], Focus());
        match(*graph[4], Hover());
        match(*graph[5], Focus());
        auto selection = select(selector, {&find_stylist(*graph[4])});
        REQUIRE(selection.size() == 2);
        REQUIRE(selection.contains(&find_stylist(*graph[3])));
        REQUIRE(selection.contains(&find_stylist(*graph[5])));
        delete graph.front();
      }
    });
  }
}
