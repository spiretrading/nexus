#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/FocusObserver.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct Observer {
    FocusObserver::State m_expected_state;
    Stylist* m_stylist;
    SelectionUpdateSignal m_on_update;
    FocusObserver m_observer;
    scoped_connection m_connection;
    bool m_is_match;

    Observer(FocusObserver::State state, const Stylist& base,
        const SelectionUpdateSignal& on_update)
        : m_expected_state(state),
          m_stylist(const_cast<Stylist*>(&base)),
          m_on_update(on_update),
          m_observer(m_stylist->get_widget()),
          m_connection(m_observer.connect_state_signal(
            std::bind_front(&Observer::on_state, this))),
          m_is_match(is_set(m_observer.get_state(), m_expected_state)) {
      if(m_is_match) {
        m_on_update({m_stylist}, {});
      }
    }

    void on_state(FocusObserver::State state) {
      auto has_focus = is_set(state, m_expected_state);
      if(m_is_match) {
        if(!has_focus) {
          m_is_match = false;
          m_on_update({}, {m_stylist});
        }
      } else if(has_focus) {
        m_is_match = true;
        m_on_update({m_stylist}, {});
      }
    }
  };
}

SelectConnection Spire::Styles::select(const Focus& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return SelectConnection(
    std::make_unique<Observer>(FocusObserver::State::FOCUS, base, on_update));
}

SelectConnection Spire::Styles::select(const FocusIn& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return SelectConnection(
    std::make_unique<Observer>(FocusObserver::State::FOCUS_IN, base, on_update));
}

SelectConnection Spire::Styles::select(const FocusVisible& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return SelectConnection(std::make_unique<Observer>(
    FocusObserver::State::FOCUS_VISIBLE, base, on_update));
}