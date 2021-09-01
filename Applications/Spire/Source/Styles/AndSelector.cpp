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

bool AndSelector::operator ==(const AndSelector& selector) const {
  return m_left == selector.get_left() && m_right == selector.get_right();
}

bool AndSelector::operator !=(const AndSelector& selector) const {
  return !(*this == selector);
}

AndSelector Spire::Styles::operator &&(Selector left, Selector right) {
  return AndSelector(std::move(left), std::move(right));
}

SelectConnection Spire::Styles::select(const AndSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  struct Executor {
    std::unordered_map<const Stylist*, int> m_selection;
    SelectionUpdateSignal m_on_update;
    SelectConnection m_left;
    SelectConnection m_right;

    Executor(const AndSelector& selector, const Stylist& base,
      const SelectionUpdateSignal& on_update)
      : m_on_update(on_update),
        m_left(select(selector.get_left(), base,
          std::bind_front(&Executor::on_update, this))),
        m_right(select(selector.get_right(), base,
          std::bind_front(&Executor::on_update, this))) {}

    void on_update(std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals) {
      for(auto i = additions.begin(); i != additions.end();) {
        auto& selection = m_selection[*i];
        ++selection;
        if(selection == 1) {
          i = additions.erase(i);
        } else {
          ++i;
        }
      }
      for(auto i = removals.begin(); i != removals.end();) {
        auto& selection = m_selection[*i];
        --selection;
        if(selection == 0) {
          i = removals.erase(i);
        } else {
          ++i;
        }
      }
      if(!additions.empty() || !removals.empty()) {
        m_on_update(std::move(additions), std::move(removals));
      }
    }
  };
  return SelectConnection(
    std::make_unique<Executor>(selector, base, on_update));
}

std::unordered_set<Stylist*> Spire::Styles::select(
    const AndSelector& selector, std::unordered_set<Stylist*> sources) {
  auto left = select(selector.get_left(), sources);
  if(left.empty()) {
    return {};
  }
  auto right = select(selector.get_right(), sources);
  if(right.empty()) {
    return {};
  }
  if(left.size() == 1 && right.size() == 1 && *left.begin() == *right.begin()) {
    return left;
  }
  for(auto i = left.begin(); i != left.end();) {
    auto match = *i;
    if(right.find(match) == right.end()) {
      i = left.erase(i);
    } else {
      ++i;
    }
  }
  return left;
}
