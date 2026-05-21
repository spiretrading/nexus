#include "Spire/Styles/ParentSelector.hpp"
#include <algorithm>
#include <ranges>
#include <boost/functional/hash.hpp>
#include <QEvent>
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  void insert_principals(
      const Stylist& root, std::unordered_set<const Stylist*>& principals) {
    for(auto& proxy : root.get_principals()) {
      principals.insert(proxy);
      insert_principals(*proxy, principals);
    }
  }

  struct ParentObserver : QObject {
    struct ParentEntry {
      QObject* m_widget;
      const Stylist* m_stylist;

      ParentEntry(QObject& widget, const Stylist& stylist)
        : m_widget(&widget),
          m_stylist(&stylist) {}
    };
    const Stylist* m_stylist;
    SelectionUpdateSignal m_on_update;
    std::vector<ParentEntry> m_parent_stylists;
    scoped_connection m_backlink_connection;

    ParentObserver(
        const Stylist& stylist, const SelectionUpdateSignal& on_update)
        : m_stylist(&stylist),
          m_on_update(on_update) {
      auto parents = std::unordered_set<const Stylist*>();
      for(auto& link : m_stylist->get_backlinks()) {
        if(add_parent(link->get_widget(), *link)) {
          parents.insert(link);
        }
      }
      m_backlink_connection = m_stylist->connect_backlink_signal(
        std::bind_front(&ParentObserver::on_backlink, this));
      m_stylist->get_widget().installEventFilter(this);
      update_parent(std::move(parents), {});
    }

    bool is_connected() const {
      return true;
    }

    bool add_parent(QObject& widget, const Stylist& stylist) {
      auto i =
        std::ranges::find(m_parent_stylists, &widget, &ParentEntry::m_widget);
      if(i != m_parent_stylists.end()) {
        return false;
      }
      m_parent_stylists.emplace_back(widget, stylist);
      return true;
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::ParentChange) {
        auto parents = std::unordered_set<const Stylist*>();
        for(auto& entry : m_parent_stylists) {
          parents.insert(entry.m_stylist);
        }
        m_parent_stylists.clear();
        update_parent({}, std::move(parents));
      }
      return QObject::eventFilter(watched, event);
    }

    void update_parent(std::unordered_set<const Stylist*>&& parents,
        std::unordered_set<const Stylist*>&& old_parents) {
      if(auto parent = find_parent(*m_stylist)) {
        add_parent(parent->get_widget(), *parent);
        parents.insert(parent);
      }
      insert_principals(*m_stylist, parents);
      if(!parents.empty() || !old_parents.empty()) {
        m_on_update(std::move(parents), std::move(old_parents));
      }
    }

    void on_backlink(const Stylist& link) {
      if(add_parent(link.get_widget(), link)) {
        m_on_update({&link}, {});
      }
    }
  };
}

ParentSelector::ParentSelector(Selector base, Selector parent)
  : m_base(std::move(base)),
    m_parent(std::move(parent)) {}

const Selector& ParentSelector::get_base() const {
  return m_base;
}

const Selector& ParentSelector::get_parent() const {
  return m_parent;
}

ParentSelector Spire::Styles::operator <(Selector base, Selector parent) {
  return ParentSelector(std::move(base), std::move(parent));
}

SelectConnection Spire::Styles::select(const ParentSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(CombinatorSelector(selector.get_base(), selector.get_parent(),
    [] (const auto& stylist, const auto& on_update) {
      return SelectConnection(
        std::make_unique<ParentObserver>(stylist, on_update));
    }), base, on_update);
}

std::size_t std::hash<ParentSelector>::operator ()(
    const ParentSelector& selector) const noexcept {
  auto seed = std::size_t(0);
  hash_combine(seed, std::hash<Selector>()(selector.get_base()));
  hash_combine(seed, std::hash<Selector>()(selector.get_parent()));
  return seed;
}
