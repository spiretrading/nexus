#include "Spire/Styles/CombinatorSelector.hpp"
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include "Spire/Styles/FlipSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

CombinatorSelector::CombinatorSelector(
  Selector base, Selector match, SelectionBuilder selection_builder)
  : m_base(std::move(base)),
    m_match(std::move(match)),
    m_selection_builder(std::move(selection_builder)) {}

CombinatorSelector::CombinatorSelector(Selector base, Selector match,
  StaticSelectionBuilder selection_builder)
  : CombinatorSelector(std::move(base), std::move(match),
      [selection_builder = std::move(selection_builder)] (
          const auto& stylist, const auto& on_update) {
        on_update(selection_builder(stylist), {});
        return SelectConnection();
      }) {}

const Selector& CombinatorSelector::get_base() const {
  return m_base;
}

const Selector& CombinatorSelector::get_match() const {
  return m_match;
}

const CombinatorSelector::SelectionBuilder&
    CombinatorSelector::get_selection_builder() const {
  return m_selection_builder;
}

SelectConnection Spire::Styles::select(const CombinatorSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  struct Executor {
    struct MatchEntry {
      int m_count;
      std::unordered_set<const Stylist*> m_selection;
      SelectConnection m_select_connection;
      scoped_connection m_delete_connection;

      MatchEntry()
        : m_count(0) {}
    };
    Selector m_match_selector;
    const Stylist* m_base;
    bool m_is_flipped;
    CombinatorSelector::SelectionBuilder m_selection_builder;
    SelectionUpdateSignal m_on_update;
    std::unordered_map<const Stylist*, SelectConnection> m_base_connections;
    std::unordered_map<const Stylist*, MatchEntry> m_match_entries;
    std::unordered_map<const Stylist*, std::unordered_set<const Stylist*>>
      m_selection;
    std::unordered_map<const Stylist*, int> m_match_counts;
    int m_matches;
    SelectConnection m_base_connection;

    Executor(const CombinatorSelector& selector, const Stylist& base,
      const SelectionUpdateSignal& on_update)
      : m_match_selector(selector.get_match()),
        m_base(&base),
        m_is_flipped(selector.get_base().get_type() == typeid(FlipSelector)),
        m_selection_builder(selector.get_selection_builder()),
        m_on_update(on_update),
        m_matches(0),
        m_base_connection(select(selector.get_base(), *m_base,
          std::bind_front(&Executor::on_base, this))) {}

    bool is_connected() const {
      return true;
    }

    void remove(const Stylist& stylist) {
      for(auto selection : m_selection) {
        if(selection.second.contains(&stylist)) {
          on_selection(*selection.first, {}, {&stylist});
        }
      }
    }

    void on_base(std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals) {
      for(auto addition : additions) {
        m_base_connections[addition] = m_selection_builder(*addition,
          std::bind_front(&Executor::on_selection, this, std::ref(*addition)));
      }
      for(auto removal : removals) {
        on_selection(*removal, {}, std::move(m_selection[removal]));
        m_selection.erase(removal);
        m_base_connections.erase(removal);
      }
    }

    void on_selection(const Stylist& base,
        std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals) {
      if(!additions.empty()) {
        m_selection[&base].insert(additions.begin(), additions.end());
      }
      auto consolidated_additions = std::unordered_set<const Stylist*>();
      auto consolidated_removals = std::unordered_set<const Stylist*>();
      auto on_update = std::move(m_on_update);
      m_on_update = [&] (auto&& additions, auto&& removals) {
        consolidated_additions.insert(additions.begin(), additions.end());
        consolidated_removals.insert(removals.begin(), removals.end());
      };
      for(auto stylist : additions) {
        auto& entry = m_match_entries[stylist];
        ++entry.m_count;
        if(entry.m_count == 1) {
          entry.m_select_connection = select(m_match_selector, *stylist,
            std::bind_front(&Executor::on_match, this, std::ref(entry)));
          entry.m_delete_connection = stylist->connect_delete_signal(
            std::bind_front(&Executor::remove, this, std::ref(*stylist)));
        }
      }
      for(auto stylist : removals) {
        auto entry = m_match_entries.find(stylist);
        if(entry == m_match_entries.end()) {
          continue;
        }
        --entry->second.m_count;
        if(entry->second.m_count == 0) {
          auto removals = std::move(entry->second.m_selection);
          entry->second.m_selection.clear();
          on_match(entry->second, {}, std::move(removals));
          m_match_entries.erase(entry);
        }
      }
      m_on_update = std::move(on_update);
      if(!consolidated_additions.empty() || !consolidated_removals.empty()) {
        m_on_update(
          std::move(consolidated_additions), std::move(consolidated_removals));
      }
    }

    void on_match(
        MatchEntry& entry, std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals) {
      enum class FlipMatch {
        NONE,
        ADD,
        REMOVE
      };
      auto flip_match = FlipMatch::NONE;
      for(auto i = additions.begin(); i != additions.end();) {
        entry.m_selection.insert(*i);
        auto& count = m_match_counts[*i];
        ++count;
        if(m_is_flipped) {
          if(count == 1) {
            ++m_matches;
            if(m_matches == 1) {
              flip_match = FlipMatch::ADD;
            }
          }
          ++i;
        } else if(count != 1) {
          i = additions.erase(i);
        } else {
          ++i;
        }
      }
      for(auto i = removals.begin(); i != removals.end();) {
        entry.m_selection.erase(*i);
        auto& count = m_match_counts[*i];
        --count;
        if(m_is_flipped) {
          if(count == 0) {
            m_match_counts.erase(*i);
            --m_matches;
            if(m_matches == 0) {
              flip_match = FlipMatch::REMOVE;
            }
          }
          ++i;
        } else if(count != 0) {
          i = removals.erase(i);
        } else {
          m_match_counts.erase(*i);
          ++i;
        }
      }
      if(m_is_flipped) {
        if(flip_match == FlipMatch::ADD) {
          m_on_update({m_base}, {});
        } else if(flip_match == FlipMatch::REMOVE) {
          m_on_update({}, {m_base});
        }
      } else if(!additions.empty() || !removals.empty()) {
        m_on_update(std::move(additions), std::move(removals));
      }
    }
  };
  return SelectConnection(
    std::make_unique<Executor>(selector, base, on_update));
}

std::size_t std::hash<CombinatorSelector>::operator ()(
    const CombinatorSelector& selector) const {
  auto seed = std::size_t(0);
  hash_combine(seed, std::hash<Selector>()(selector.get_base()));
  hash_combine(seed, std::hash<Selector>()(selector.get_match()));
  return seed;
}
