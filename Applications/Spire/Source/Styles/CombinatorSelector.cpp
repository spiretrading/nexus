#include "Spire/Styles/CombinatorSelector.hpp"
#include <unordered_map>
#include "Spire/Styles/FlipSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

CombinatorSelector::CombinatorSelector(
  Selector base, Selector match, SelectionBuilder selection_builder)
  : m_base(std::move(base)),
    m_match(std::move(match)),
    m_selection_builder(std::move(selection_builder)) {}

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

bool CombinatorSelector::operator ==(const CombinatorSelector& selector) const {
  return m_base == selector.get_base() && m_match == selector.get_match();
}

bool CombinatorSelector::operator !=(const CombinatorSelector& selector) const {
  return !(*this == selector);
}

SelectConnection Spire::Styles::select(const CombinatorSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  struct Executor {
    struct MatchEntry {
      int m_count;
      SelectConnection m_connection;
      std::unordered_set<const Stylist*> m_selection;

      MatchEntry()
        : m_count(0) {}
    };
    Selector m_match_selector;
    const Stylist* m_base;
    bool m_is_flipped;
    CombinatorSelector::SelectionBuilder m_selection_builder;
    SelectionUpdateSignal m_on_update;
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

    void on_base(std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals) {
      for(auto addition : additions) {
        auto selection = m_selection_builder(*addition);
        m_selection[addition].insert(selection.begin(), selection.end());
        for(auto stylist : selection) {
          auto& entry = m_match_entries[stylist];
          ++entry.m_count;
          if(entry.m_count == 1) {
            entry.m_connection = select(m_match_selector, *stylist,
              std::bind_front(&Executor::on_match, this, std::ref(entry)));
          }
        }
      }
      for(auto removal : removals) {
        for(auto selection : m_selection[removal]) {
          auto& entry = m_match_entries[selection];
          --entry.m_count;
          if(entry.m_count == 0) {
            auto removals = std::move(entry.m_selection);
            entry.m_selection.clear();
            on_match(entry, {}, std::move(removals));
            m_match_entries.erase(selection);
          }
        }
        m_selection.erase(removal);
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
            --m_matches;
            if(m_matches == 0) {
              flip_match = FlipMatch::REMOVE;
            }
          }
          ++i;
        } else if(count != 0) {
          i = removals.erase(i);
        } else {
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
