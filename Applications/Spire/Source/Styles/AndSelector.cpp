#include "Spire/Styles/AndSelector.hpp"
#include <unordered_set>

using namespace Spire;
using namespace Spire::Styles;

AndSelector::AndSelector(Selector left, Selector right)
  : m_left(std::move(left)),
    m_right(std::move(right)) {}

const Selector& AndSelector::get_left() const {
  return m_left;
}

const Selector& AndSelector::get_right() const {
  return m_right;
}

bool AndSelector::is_match(const AndSelector& selector) const {
  return m_left.is_match(selector.get_left()) &&
    m_right.is_match(selector.get_right());
}

AndSelector Spire::Styles::operator &&(Selector left, Selector right) {
  return AndSelector(std::move(left), std::move(right));
}

std::vector<QWidget*> Spire::Styles::select(
    const AndSelector& selector, QWidget& source) {
  auto left_selection = select(selector.get_left(), source);
  auto right_selection = select(selector.get_right(), source);
  if(left_selection.empty() || right_selection.empty()) {
    return {};
  }
  if(left_selection.size() == 1 && right_selection.size() == 1) {
    if(left_selection.front() == right_selection.front()) {
      return left_selection;
    }
    return {};
  }
  auto& [small, big] = [&] () {
    if(left_selection.size() < right_selection.size()) {
      return std::tie(left_selection, right_selection);
    }
    return std::tie(right_selection, left_selection);
  }();
  auto small_set = std::unordered_set(small.begin(), small.end());
  auto selection = std::vector<QWidget*>();
  for(auto& candidate : big) {
    if(small_set.find(candidate) != small_set.end()) {
      selection.push_back(candidate);
    }
  }
  return selection;
}
