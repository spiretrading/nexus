#include "Spire/Styles/ParentSelector.hpp"
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

  struct ParentObserver : public QObject {
    const Stylist* m_stylist;
    SelectionUpdateSignal m_on_update;
    std::unordered_map<QObject*, const Stylist*> m_parent_stylists;
    scoped_connection m_backlink_connection;

    ParentObserver(
        const Stylist& stylist, const SelectionUpdateSignal& on_update)
        : m_stylist(&stylist),
          m_on_update(on_update) {
      auto parents = std::unordered_set<const Stylist*>();
      for(auto& link : m_stylist->get_backlinks()) {
        if(m_parent_stylists.insert(
            std::pair(&link->get_widget(), link)).second) {
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

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::ParentChange) {
        auto& widget = static_cast<QWidget&>(*watched);
        auto parents = std::unordered_set<const Stylist*>();
        for(auto& parent : m_parent_stylists | std::views::values) {
          parents.insert(parent);
        }
        m_parent_stylists.clear();
        update_parent({}, std::move(parents));
      }
      return QObject::eventFilter(watched, event);
    }

    void update_parent(std::unordered_set<const Stylist*>&& parents,
        std::unordered_set<const Stylist*>&& old_parents) {
      if(auto parent = find_parent(*m_stylist)) {
        m_parent_stylists.insert(std::pair(&parent->get_widget(), parent));
        parents.insert(parent);
      }
      insert_principals(*m_stylist, parents);
      if(!parents.empty() || !old_parents.empty()) {
        m_on_update(std::move(parents), std::move(old_parents));
      }
    }

    void on_backlink(const Stylist& link) {
      if(m_parent_stylists.insert(
          std::pair(&link.get_widget(), &link)).second) {
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

SelectConnection Spire::Styles::select(const ParentSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(CombinatorSelector(selector.get_base(), selector.get_parent(),
    [] (const auto& stylist, const auto& on_update) {
      return SelectConnection(
        std::make_unique<ParentObserver>(stylist, on_update));
    }), base, on_update);
}

std::size_t std::hash<ParentSelector>::operator ()(
    const ParentSelector& selector) const {
  auto seed = std::size_t(0);
  hash_combine(seed, std::hash<Selector>()(selector.get_base()));
  hash_combine(seed, std::hash<Selector>()(selector.get_parent()));
  return seed;
}
