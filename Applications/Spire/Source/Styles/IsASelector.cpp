#include "Spire/Styles/IsASelector.hpp"
#include <QEvent>
#include <QTimer>
#include "Spire/Styles/Stylist.hpp"

using namespace boost::signals2;
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
    return {};
  } else if(base.get_widget().testAttribute(Qt::WA_WState_Polished)) {
    return {};
  }
  struct Executor : QObject {
    IsASelector m_selector;
    Stylist* m_stylist;
    SelectionUpdateSignal m_on_update;
    scoped_connection m_delete_connection;

    Executor(const IsASelector& selector, const Stylist& base,
        const SelectionUpdateSignal& on_update)
        : m_selector(selector),
          m_stylist(&const_cast<Stylist&>(base)),
          m_on_update(on_update) {
      m_delete_connection = m_stylist->connect_delete_signal(
        std::bind_front(&Executor::on_delete, this));
      QTimer::singleShot(0, this, std::bind_front(&Executor::test, this));
    }

    bool is_connected() const {
      return m_stylist != nullptr;
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::Polish) {
        auto stylist = std::exchange(m_stylist, nullptr);
        stylist->get_widget().removeEventFilter(this);
        if(m_selector.is_instance(stylist->get_widget())) {
          m_on_update({stylist}, {});
        } else {
          m_on_update({}, {});
        }
      }
      return QObject::eventFilter(watched, event);
    }

    void test() {
      if(!m_delete_connection.connected()) {
        return;
      }
      if(m_selector.is_instance(m_stylist->get_widget())) {
        auto stylist = std::exchange(m_stylist, nullptr);
        m_on_update({stylist}, {});
      } else {
        m_stylist->get_widget().installEventFilter(this);
      }
    }

    void on_delete() {
      m_delete_connection.disconnect();
    }
  };
  return SelectConnection(
    std::make_unique<Executor>(selector, base, on_update));
}

std::size_t std::hash<IsASelector>::operator ()(
    const IsASelector& selector) const {
  return std::hash<std::type_index>()(selector.get_type());
}
