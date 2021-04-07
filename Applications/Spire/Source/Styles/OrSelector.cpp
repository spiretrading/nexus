#include "Spire/Styles/OrSelector.hpp"
#include <unordered_set>

using namespace Spire;
using namespace Spire::Styles;

OrSelector::OrSelector(Selector left, Selector right)
  : m_left(std::move(left)),
    m_right(std::move(right)) {}

const Selector& OrSelector::get_left() const {
  return m_left;
}

const Selector& OrSelector::get_right() const {
  return m_right;
}

bool OrSelector::is_match(const OrSelector& selector) const {
  return m_left.is_match(selector.get_left()) &&
    m_right.is_match(selector.get_right());
}

OrSelector Spire::Styles::operator ||(Selector left, Selector right) {
  return OrSelector(std::move(left), std::move(right));
}

std::vector<QWidget*> Spire::Styles::select(
    const OrSelector& selector, QWidget& source) {
  auto left_selection = select(selector.get_left(), source);
  if(left_selection.empty()) {
    return select(selector.get_right(), source);
  }
  auto right_selection = select(selector.get_right(), source);
  if(right_selection.empty()) {
    return left_selection;
  }
  if(left_selection.size() == 1 && right_selection.size() == 1) {
    if(left_selection.front() != right_selection.front()) {
      left_selection.push_back(right_selection.front());
    }
    return left_selection;
  }
  auto result =
    std::unordered_set(left_selection.begin(), left_selection.end());
  result.insert(right_selection.begin(), right_selection.end());
  return std::vector(result.begin(), result.end());
}
