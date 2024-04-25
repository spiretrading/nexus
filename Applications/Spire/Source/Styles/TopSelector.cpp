#include "Spire/Styles/TopSelector.hpp"
#include <boost/functional/hash.hpp>
#include <QChildEvent>
#include <QWidget>
#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  void erase_common(std::unordered_set<const Stylist*>& a,
      std::unordered_set<const Stylist*>& b) {
    auto [begin, end, set] = [&] {
      if(a.size() < b.size()) {
        return std::tuple(a.begin(), a.end(), &b);
      }
      return std::tuple(b.begin(), b.end(), &a);
    }();
    auto intersection = std::unordered_set<const Stylist*>();
    std::copy_if(begin, end, std::inserter(intersection, intersection.begin()),
      [&] (const auto& element) { return set->count(element) > 0; });
    for(auto i : intersection) {
      a.erase(i);
      b.erase(i);
    }
  }

  struct TopObserver : public QObject {
    struct Match {
      SelectConnection m_connection;
      std::unordered_set<const Stylist*> m_selection;
    };
    Selector m_selector;
    const Stylist* m_stylist;
    SelectionUpdateSignal m_on_update;
    SelectConnection m_select_connection;
    std::unordered_set<const Stylist*> m_base_selection;
    std::unordered_set<const Stylist*> m_children_selection;
    std::unordered_map<const Stylist*, int> m_selection_count;
    std::unordered_map<const Stylist*, Match> m_matches;
    std::unordered_map<QObject*, const Stylist*> m_children_stylists;

    TopObserver(const Selector& selector, const Stylist& stylist,
        const SelectionUpdateSignal& on_update)
        : m_selector(selector),
          m_stylist(&stylist),
          m_on_update(on_update) {
      m_select_connection = select(
        m_selector, *m_stylist, std::bind_front(&TopObserver::on_update, this));
      if(m_select_connection.is_connected() || m_base_selection.empty()) {
        for(auto child : m_stylist->get_widget().children()) {
          if(child && child->isWidgetType()) {
            add(find_stylist(*static_cast<QWidget*>(child)));
          }
        }
        m_stylist->get_widget().installEventFilter(this);
      }
    }

    bool is_connected() const {
      return m_select_connection.is_connected() || m_base_selection.empty();
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::ChildAdded) {
        auto& child = *static_cast<QChildEvent&>(*event).child();
        if(child.isWidgetType()) {
          add(find_stylist(static_cast<QWidget&>(child)));
        }
      } else if(event->type() == QEvent::ChildRemoved) {
        auto& child = *static_cast<QChildEvent&>(*event).child();
        auto i = m_children_stylists.find(&child);
        if(i != m_children_stylists.end()) {
          remove(static_cast<QWidget&>(child));
        }
      }
      return QObject::eventFilter(watched, event);
    }

    void on_update(std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals) {
      if(m_base_selection.empty()) {
        m_base_selection = additions;
        auto children_removals = m_children_selection;
        erase_common(additions, children_removals);
        m_on_update(std::move(additions), std::move(children_removals));
      } else {
        for(auto removal : removals) {
          m_base_selection.erase(removal);
        }
        m_base_selection.insert(additions.begin(), additions.end());
        if(m_base_selection.empty()) {
          auto children_selection = m_children_selection;
          for(auto selection : children_selection) {
            removals.erase(selection);
          }
          m_on_update(std::move(children_selection), std::move(removals));
        } else {
          m_on_update(std::move(additions), std::move(removals));
        }
      }
      if(!m_select_connection.is_connected()) {
        m_select_connection.disconnect();
        if(m_base_selection.empty()) {
        } else {
          for(auto child : m_stylist->get_widget().children()) {
            if(child && child->isWidgetType()) {
              remove(static_cast<QWidget&>(*child));
            }
          }
        }
      }
    }

    void on_child_update(Match& match,
        std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals) {
      for(auto removal : removals) {
        match.m_selection.erase(removal);
      }
      match.m_selection.insert(additions.begin(), additions.end());
      std::erase_if(additions, [&] (auto addition) {
        auto& count = m_selection_count[addition];
        ++count;
        return count != 1;
      });
      m_children_selection.insert(additions.begin(), additions.end());
      if(m_base_selection.empty()) {
        std::erase_if(removals, [&] (auto removal) {
          auto& count = m_selection_count[removal];
          --count;
          if(count == 0) {
            m_selection_count.erase(removal);
            m_children_selection.erase(removal);
            return false;
          }
          return true;
        });
        m_on_update(std::move(additions), std::move(removals));
      } else {
        for(auto removal : removals) {
          auto& count = m_selection_count[removal];
          --count;
          if(count == 0) {
            m_selection_count.erase(removal);
            m_children_selection.erase(removal);
          }
        }
      }
    }

    void add(const Stylist& stylist) {
      if(m_matches.contains(&stylist)) {
        return;
      }
      m_children_stylists.insert(std::pair(&stylist.get_widget(), &stylist));
      auto& match = m_matches[&stylist];
      match.m_connection = select(TopSelector(Any(), m_selector), stylist,
        std::bind_front(&TopObserver::on_child_update, this, std::ref(match)));
    }

    void remove(QWidget& widget) {
      auto i = m_children_stylists.find(&widget);
      if(i == m_children_stylists.end()) {
        return;
      }
      auto& stylist = *i->second;
      m_children_stylists.erase(i);
      auto j = m_matches.find(&stylist);
      if(j == m_matches.end()) {
        return;
      }
      auto& match = j->second;
      auto removals = match.m_selection;
      on_child_update(match, {}, std::move(removals));
      m_matches.erase(j);
    }
  };
}

TopSelector Spire::Styles::operator >(Selector base, Selector child) {
  return TopSelector(std::move(base), std::move(child));
}

TopSelector::TopSelector(Selector base, Selector descendant)
  : m_base(std::move(base)),
    m_descendant(std::move(descendant)) {}

const Selector& TopSelector::get_base() const {
  return m_base;
}

const Selector& TopSelector::get_descendant() const {
  return m_descendant;
}

SelectConnection Spire::Styles::select(const TopSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(CombinatorSelector(selector.get_base(), Any(),
    [=] (const auto& stylist, const auto& on_update) {
      return SelectConnection(std::make_unique<TopObserver>(
        selector.get_descendant(), stylist, on_update));
    }), base, on_update);
}

std::size_t std::hash<TopSelector>::operator ()(
    const TopSelector& selector) const {
  auto seed = std::size_t(0);
  hash_combine(seed, std::hash<Selector>()(selector.get_base()));
  hash_combine(seed, std::hash<Selector>()(selector.get_descendant()));
  return seed;
}
