#include "Spire/Styles/OrSelector.hpp"
#include <boost/functional/hash.hpp>

using namespace boost;
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

OrSelector Spire::Styles::operator ||(Selector left, Selector right) {
  return OrSelector(std::move(left), std::move(right));
}

SelectConnection Spire::Styles::select(const OrSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  struct Executor {
    std::unordered_map<const Stylist*, int> m_selection;
    SelectionUpdateSignal m_on_update;
    SelectConnection m_left;
    SelectConnection m_right;

    Executor(const OrSelector& selector, const Stylist& base,
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
        if(selection != 1) {
          i = additions.erase(i);
        } else {
          ++i;
        }
      }
      for(auto i = removals.begin(); i != removals.end();) {
        auto& selection = m_selection[*i];
        --selection;
        if(selection != 0) {
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

std::size_t std::hash<OrSelector>::operator ()(
    const OrSelector& selector) const {
  auto seed = std::size_t(0);
  hash_combine(seed, std::hash<Selector>()(selector.get_left()));
  hash_combine(seed, std::hash<Selector>()(selector.get_right()));
  return seed;
}
