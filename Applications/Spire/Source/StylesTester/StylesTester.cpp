#include "Spire/StylesTester/StylesTester.hpp"
#include <doctest/doctest.h>
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

std::unordered_map<std::string, QWidget*> Spire::Styles::Tests::make_graph() {
  auto nodes = std::unordered_map<std::string, QWidget*>();
  nodes["A"] = new QWidget();
  nodes["B"] = new QWidget(nodes["A"]);
  nodes["C"] = new QWidget(nodes["B"]);
  nodes["D"] = new QWidget(nodes["B"]);
  nodes["E"] = new QWidget(nodes["C"]);
  nodes["F"] = new QWidget(nodes["C"]);
  nodes["G"] = new QWidget(nodes["D"]);
  nodes["H"] = new QWidget(nodes["D"]);
  return nodes;
}

void Spire::Styles::Tests::require_selection(
    std::deque<SelectionUpdate>& updates,
    const std::unordered_map<std::string, QWidget*>& graph,
    const std::unordered_set<std::string>& expected_additions,
    const std::unordered_set<std::string>& expected_removals) {
  if(expected_additions.empty() && expected_removals.empty()) {
    REQUIRE(updates.empty());
    return;
  }
  REQUIRE(updates.size() == 1);
  auto matches = updates.front();
  updates.pop_front();
  REQUIRE(matches.m_additions.size() == expected_additions.size());
  REQUIRE(matches.m_removals.size() == expected_removals.size());
  for(auto& addition : expected_additions) {
    REQUIRE(matches.m_additions.contains(&find_stylist(*graph.at(addition))));
  }
  for(auto& removal : expected_removals) {
    REQUIRE(matches.m_removals.contains(&find_stylist(*graph.at(removal))));
  }
}
