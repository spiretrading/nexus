#include "Spire/Styles/DescendantSelector.hpp"
#include <unordered_map>
#include <unordered_set>
#include <boost/functional/hash.hpp>
#include <QChildEvent>
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct Executor : public QObject {
    struct Descendant {
      const Stylist* m_stylist;
      scoped_connection m_delete_connection;
    };
    SelectionUpdateSignal m_on_update;
    std::unordered_map<const QObject*, Descendant> m_descendants;

    Executor(const Stylist& stylist, const SelectionUpdateSignal& on_update)
        : m_on_update(on_update) {
      stylist.get_widget().installEventFilter(this);
      auto descendants = std::unordered_set<const Stylist*>();
      add_children(stylist.get_widget(), descendants);
      m_on_update(std::move(descendants), {});
    }

    bool is_connected() const {
      return true;
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::ChildAdded) {
        auto& child = *static_cast<QChildEvent&>(*event).child();
        if(child.isWidgetType()) {
          auto descendants = std::unordered_set<const Stylist*>();
          add(static_cast<QWidget&>(child), descendants);
          if(!descendants.empty()) {
            m_on_update(std::move(descendants), {});
          }
        }
      } else if(event->type() == QEvent::ChildRemoved) {
        auto descendants = std::unordered_set<const Stylist*>();
        remove(*static_cast<QChildEvent&>(*event).child(), descendants);
        if(!descendants.empty()) {
          m_on_update({}, std::move(descendants));
        }
      }
      return QObject::eventFilter(watched, event);
    }

    void add(QWidget& widget, std::unordered_set<const Stylist*>& descendants) {
      if(m_descendants.contains(&widget)) {
        return;
      }
      auto& stylist = find_stylist(widget);
      m_descendants.try_emplace(
        &widget, &stylist, stylist.connect_delete_signal(
          std::bind_front(&Executor::on_delete, this, std::ref(stylist))));
      widget.installEventFilter(this);
      descendants.insert(&stylist);
      add_children(widget, descendants);
    }

    void add_children(
        QWidget& widget, std::unordered_set<const Stylist*>& descendants) {
      for(auto child : widget.children()) {
        if(child && child->isWidgetType()) {
          add(static_cast<QWidget&>(*child), descendants);
        }
      }
    }

    void remove(
        QObject& object, std::unordered_set<const Stylist*>& descendants) {
      auto i = m_descendants.find(&object);
      if(i == m_descendants.end()) {
        return;
      }
      descendants.insert(i->second.m_stylist);
      m_descendants.erase(i);
      object.removeEventFilter(this);
      for(auto child : object.children()) {
        if(child) {
          remove(*child, descendants);
        }
      }
    }

    void on_delete(const Stylist& stylist) {
      auto descendants = std::unordered_set<const Stylist*>();
      remove(stylist.get_widget(), descendants);
      if(!descendants.empty()) {
        m_on_update({}, std::move(descendants));
      }
    }
  };
}

DescendantSelector::DescendantSelector(Selector base, Selector descendant)
  : m_base(std::move(base)),
    m_descendant(std::move(descendant)) {}

const Selector& DescendantSelector::get_base() const {
  return m_base;
}

const Selector& DescendantSelector::get_descendant() const {
  return m_descendant;
}

DescendantSelector Spire::Styles::operator >>(
    Selector base, Selector descendant) {
  return DescendantSelector(std::move(base), std::move(descendant));
}

SelectConnection Spire::Styles::select(const DescendantSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(CombinatorSelector(selector.get_base(),
    selector.get_descendant(), [] (const auto& stylist, const auto& on_update) {
      return SelectConnection(std::make_unique<Executor>(stylist, on_update));
    }), base, on_update);
}

std::size_t std::hash<DescendantSelector>::operator ()(
    const DescendantSelector& selector) const noexcept {
  auto seed = std::size_t(0);
  hash_combine(seed, std::hash<Selector>()(selector.get_base()));
  hash_combine(seed, std::hash<Selector>()(selector.get_descendant()));
  return seed;
}
