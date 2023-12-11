#include "Spire/Styles/IsASelector.hpp"
#include <QEvent>
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

const std::type_index& IsASelector::get_type() const {
  return m_type;
}

bool IsASelector::is_instance(const QWidget& widget) const {
  return m_is_instance(widget);
}

bool IsASelector::operator ==(const IsASelector& selector) const {
  return m_type == selector.get_type();
}

SelectConnection Spire::Styles::select(const IsASelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  if(base.get_pseudo_element()) {
    return {};
  } else if(selector.is_instance(base.get_widget())) {
    on_update({&base}, {});
  } else if(base.get_widget().testAttribute(Qt::WA_WState_Polished)) {
    return {};
  }
  struct Executor : QObject {
    IsASelector m_selector;
    Stylist* m_stylist;
    SelectionUpdateSignal m_on_update;

    Executor(const IsASelector& selector, const Stylist& base,
        const SelectionUpdateSignal& on_update)
        : m_selector(selector),
          m_stylist(&const_cast<Stylist&>(base)),
          m_on_update(on_update) {
      auto& widget = m_stylist->get_widget();
      widget.installEventFilter(this);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::Polish) {
        if(m_selector.is_instance(m_stylist->get_widget())) {
          m_on_update({m_stylist}, {});
        }
        m_stylist->get_widget().removeEventFilter(this);
      }
      return QObject::eventFilter(watched, event);
    }
  };
  return SelectConnection(
    std::make_unique<Executor>(selector, base, on_update));
}

std::size_t std::hash<IsASelector>::operator ()(
    const IsASelector& selector) const {
  return std::hash<std::type_index>()(selector.get_type());
}
