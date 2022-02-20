#include "Spire/Styles/TopSelector.hpp"
#include <QChildEvent>
#include <QWidget>
#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/FlipSelector.hpp"
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
    a.erase(intersection.begin(), intersection.end());
    b.erase(intersection.begin(), intersection.end());
  }

  struct TopObserver : public QObject {
    Selector m_selector;
    const Stylist* m_stylist;
    SelectionUpdateSignal m_on_update;
    std::unordered_set<const Stylist*> m_base_selection;
    std::unordered_set<const Stylist*> m_children_selection;
    std::unordered_map<const Stylist*, SelectConnection> m_connections;

    TopObserver(const Selector& selector, const Stylist& stylist,
        const SelectionUpdateSignal& on_update)
        : m_selector(selector),
          m_stylist(&stylist),
          m_on_update(on_update) {
      m_connections.insert(std::pair(&stylist, select(m_selector, *m_stylist,
        std::bind_front(&TopObserver::on_update, this))));
      for(auto child : stylist.get_widget().children()) {
        if(child->isWidgetType()) {
          auto& stylist = find_stylist(*static_cast<QWidget*>(child));
          m_connections.insert(std::pair(
            &stylist, select(TopSelector(m_selector, m_selector), stylist,
              std::bind_front(&TopObserver::on_child_update, this))));
        }
      }
      stylist.get_widget().installEventFilter(this);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::ChildAdded) {
        auto& child_event = static_cast<QChildEvent&>(*event);
        if(child_event.child()->isWidgetType()) {
          auto& stylist =
            find_stylist(*static_cast<QWidget*>(child_event.child()));
          m_connections.insert(std::pair(&stylist,
            select(TopSelector(m_selector, m_selector), stylist,
              std::bind_front(&TopObserver::on_child_update, this))));
        }
      } else if(event->type() == QEvent::ChildRemoved) {
        auto& child_event = static_cast<QChildEvent&>(*event);
        if(child_event.child()->isWidgetType()) {
          auto& stylist =
            find_stylist(*static_cast<QWidget*>(child_event.child()));
          m_connections.erase(&stylist);
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
        return;
      }
      m_base_selection.insert(additions.begin(), additions.end());
      m_base_selection.erase(removals.begin(), removals.end());
      if(m_base_selection.empty()) {
        auto children_selection = m_children_selection;
        removals.erase(children_selection.begin(), children_selection.end());
        m_on_update(std::move(children_selection), std::move(removals));
      } else {
        m_on_update(std::move(additions), std::move(removals));
      }
    }

    void on_child_update(std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals) {
      m_children_selection.insert(additions.begin(), additions.end());
      m_children_selection.erase(removals.begin(), removals.end());
      if(m_base_selection.empty()) {
        m_on_update(std::move(additions), std::move(removals));
      }
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
