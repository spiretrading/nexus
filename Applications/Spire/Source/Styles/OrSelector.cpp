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
        const SelectionUpdateSignal& on_update) {
      auto additions = std::unordered_set<const Stylist*>();
      m_on_update = [&] (auto&& update_additions, auto&&) {
        additions.insert(update_additions.begin(), update_additions.end());
      };
      m_left = select(selector.get_left(), base,
        std::bind_front(&Executor::on_update, this));
      m_right = select(selector.get_right(), base,
        std::bind_front(&Executor::on_update, this));
      m_on_update = on_update;
      if(!additions.empty()) {
        m_on_update(std::move(additions), {});
      }
    }

    bool is_connected() const {
      return m_left.is_connected() || m_right.is_connected();
    }

    void on_update(std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals) {
      std::erase_if(additions, [&] (const auto& addition) {
        auto& selection = m_selection[addition];
        ++selection;
        return selection != 1;
      });
      std::erase_if(removals, [&] (const auto& removal) {
        auto& selection = m_selection[removal];
        --selection;
        return selection != 0;
      });
      if(!m_left.is_connected()) {
        m_left.disconnect();
      }
      if(!m_right.is_connected()) {
        m_right.disconnect();
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
