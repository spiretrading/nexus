#include "Spire/Styles/SiblingSelector.hpp"
#include <boost/functional/hash.hpp>
#include <QEvent>
#include <QWidget>
#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/ChildSelector.hpp"
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct SiblingObserver : public QObject {
    const Stylist* m_stylist;
    SelectionUpdateSignal m_on_update;
    SelectConnection m_parent_connection;
    std::unordered_set<const Stylist*> m_selection;

    SiblingObserver(
        const Stylist& stylist, const SelectionUpdateSignal& on_update)
        : m_stylist(&stylist),
          m_on_update(on_update) {
      connect_parent();
      stylist.get_widget().installEventFilter(this);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::ParentChange) {
        m_parent_connection = {};
        connect_parent();
      }
      return QObject::eventFilter(watched, event);
    }

    void connect_parent() {
      if(auto parent = m_stylist->get_widget().parentWidget()) {
        m_parent_connection =
          select(ChildSelector(Any(), Any()), find_stylist(*parent),
            std::bind_front(&SiblingObserver::on_selection, this));
      } else {
        auto selection = std::move(m_selection);
        m_selection.clear();
        m_on_update({}, std::move(selection));
      }
    }

    void on_selection(std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals) {
      if(additions.erase(m_stylist) == 0) {
        removals.erase(m_stylist);
      }
      if(additions.empty() && removals.empty()) {
        return;
      }
      if(!m_parent_connection.is_connected()) {
        auto selection = std::move(m_selection);
        m_selection.clear();
        m_selection.insert(additions.begin(), additions.end());
        m_on_update(std::move(additions), std::move(selection));
      } else {
        m_selection.erase(removals.begin(), removals.end());
        m_selection.insert(additions.begin(), additions.end());
        m_on_update(std::move(additions), std::move(removals));
      }
    }
  };
}

SiblingSelector::SiblingSelector(Selector base, Selector sibling)
  : m_base(std::move(base)),
    m_sibling(std::move(sibling)) {}

const Selector& SiblingSelector::get_base() const {
  return m_base;
}

const Selector& SiblingSelector::get_sibling() const {
  return m_sibling;
}

SiblingSelector Spire::Styles::operator %(Selector base, Selector sibling) {
  return SiblingSelector(std::move(base), std::move(sibling));
}

SelectConnection Spire::Styles::select(const SiblingSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(CombinatorSelector(selector.get_base(), selector.get_sibling(),
    [] (const auto& stylist, const auto& on_update) {
      return SelectConnection(
        std::make_unique<SiblingObserver>(stylist, on_update));
    }), base, on_update);
}

std::size_t std::hash<SiblingSelector>::operator ()(
    const SiblingSelector& selector) {
  auto seed = std::size_t(0);
  hash_combine(seed, std::hash<Selector>()(selector.get_base()));
  hash_combine(seed, std::hash<Selector>()(selector.get_sibling()));
  return seed;
}
