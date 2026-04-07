#include "Spire/Styles/AndSelector.hpp"
#include <ranges>
#include <unordered_map>
#include <boost/functional/hash.hpp>

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto LEFT_FLAG = 1;
  const auto RIGHT_FLAG = 2;
  const auto SELECTED_FLAG = LEFT_FLAG | RIGHT_FLAG;
}

AndSelector::AndSelector(Selector left, Selector right)
  : m_left(std::move(left)),
    m_right(std::move(right)) {}

const Selector& AndSelector::get_left() const {
  return m_left;
}

const Selector& AndSelector::get_right() const {
  return m_right;
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
    bool m_is_initialized;

    Executor(const AndSelector& selector, const Stylist& base,
        const SelectionUpdateSignal& on_update)
        : m_is_initialized(false) {
      m_left = select(selector.get_left(), base,
        std::bind_front(&Executor::on_update, this, LEFT_FLAG));
      if(!m_left.is_connected() && m_selection.empty()) {
        return;
      }
      auto additions = std::unordered_set<const Stylist*>();
      m_on_update = [&] (auto&& initial, auto&&) {
        additions = std::move(initial);
      };
      m_right = select(selector.get_right(), base,
        std::bind_front(&Executor::on_update, this, RIGHT_FLAG));
      m_on_update = on_update;
      m_is_initialized = true;
      if(!additions.empty()) {
        m_on_update(std::move(additions), {});
      }
    }

    bool is_connected() const {
      return m_left.is_connected() || m_right.is_connected();
    }

    void test_connection(SelectConnection& base,
        SelectConnection& connection, int flag) {
      if(base.is_connected()) {
        return;
      }
      base.disconnect();
      if(!m_is_initialized) {
        return;
      }
      auto values = m_selection | std::views::values;
      auto i = std::ranges::find_if(values, [&] (auto count) {
        return count & flag;
      });
      if(i == values.end()) {
        m_selection = {};
        connection.disconnect();
      }
    }

    void on_update(int flag, std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals) {
      std::erase_if(additions, [&] (const auto& addition) {
        auto& selection = m_selection[addition];
        selection |= flag;
        return selection != SELECTED_FLAG;
      });
      std::erase_if(removals, [&] (const auto& removal) {
        auto& selection = m_selection[removal];
        selection &= ~flag;
        return selection == 0;
      });
      test_connection(m_left, m_right, LEFT_FLAG);
      test_connection(m_right, m_left, RIGHT_FLAG);
      if(!additions.empty() || !removals.empty()) {
        m_on_update(std::move(additions), std::move(removals));
      }
    }
  };
  return SelectConnection(
    std::make_unique<Executor>(selector, base, on_update));
}

std::size_t std::hash<AndSelector>::operator ()(
    const AndSelector& selector) const noexcept {
  auto seed = std::size_t(0);
  hash_combine(seed, std::hash<Selector>()(selector.get_left()));
  hash_combine(seed, std::hash<Selector>()(selector.get_right()));
  return seed;
}
