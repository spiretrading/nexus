#include "Spire/Styles/ParentSelector.hpp"
#include <boost/functional/hash.hpp>
#include <QEvent>
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct ParentObserver : public QObject {
    SelectionUpdateSignal m_on_update;
    QWidget* m_parent;

    ParentObserver(
        const Stylist& stylist, const SelectionUpdateSignal& on_update)
        : m_on_update(on_update) {
      if(auto parent = find_parent(stylist)) {
        m_parent = &parent->get_widget();
        m_on_update({parent}, {});
      } else {
        m_parent = nullptr;
      }
      stylist.get_widget().installEventFilter(this);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::ParentChange) {
        auto& widget = static_cast<QWidget&>(*watched);
        auto old_parent = m_parent;
        if(auto parent = find_parent(find_stylist(widget))) {
          m_parent = &parent->get_widget();
        } else {
          m_parent = nullptr;
        }
        if(m_parent && old_parent) {
          m_on_update({&find_stylist(*m_parent)}, {&find_stylist(*old_parent)});
        } else if(m_parent) {
          m_on_update({&find_stylist(*m_parent)}, {});
        } else {
          m_on_update({}, {&find_stylist(*old_parent)});
        }
      }
      return QObject::eventFilter(watched, event);
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
