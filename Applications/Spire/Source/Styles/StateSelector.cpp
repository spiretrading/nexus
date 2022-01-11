#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/HoverObserver.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct FocusExecutor {
    FocusObserver::State m_expected_state;
    Stylist* m_stylist;
    SelectionUpdateSignal m_on_update;
    FocusObserver m_observer;
    bool m_is_match;
    scoped_connection m_connection;

    FocusExecutor(FocusObserver::State state, const Stylist& base,
        const SelectionUpdateSignal& on_update)
        : m_expected_state(state),
          m_stylist(const_cast<Stylist*>(&base)),
          m_on_update(on_update),
          m_observer(m_stylist->get_widget()),
          m_is_match(false),
          m_connection(m_observer.connect_state_signal(
            std::bind_front(&FocusExecutor::on_state, this))) {
      if(is_match(m_observer.get_state())) {
        m_is_match = true;
        m_on_update({m_stylist}, {});
      }
    }

    void on_state(FocusObserver::State state) {
      if(is_match(state) && !m_is_match) {
        m_is_match = true;
        m_on_update({m_stylist}, {});
      } else if(m_is_match) {
        m_is_match = false;
        m_on_update({}, {m_stylist});
      }
    }

    bool is_match(FocusObserver::State state) const {
      return state == m_expected_state ||
        m_expected_state == FocusObserver::State::FOCUS &&
        is_set(state, m_expected_state);
    }
  };

  struct HoverExecutor {
    Stylist* m_stylist;
    SelectionUpdateSignal m_on_update;
    HoverObserver m_observer;
    scoped_connection m_connection;
    bool m_is_match;

    HoverExecutor(const Stylist& base, const SelectionUpdateSignal& on_update)
        : m_stylist(const_cast<Stylist*>(&base)),
          m_on_update(on_update),
          m_observer(m_stylist->get_widget()),
          m_connection(m_observer.connect_state_signal(
            std::bind_front(&HoverExecutor::on_state, this))),
          m_is_match(
            is_set(m_observer.get_state(), HoverObserver::State::MOUSE_OVER)) {
      if(m_is_match) {
        m_on_update({m_stylist}, {});
      }
    }

    void on_state(HoverObserver::State state) {
      auto is_hovered = is_set(state, HoverObserver::State::MOUSE_OVER);
      if(m_is_match) {
        if(!is_hovered) {
          m_is_match = false;
          m_on_update({}, {m_stylist});
        }
      } else if(is_hovered) {
        m_is_match = true;
        m_on_update({m_stylist}, {});
      }
    }
  };
}

SelectConnection Spire::Styles::select(const Focus& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return SelectConnection(std::make_unique<FocusExecutor>(
    FocusObserver::State::FOCUS, base, on_update));
}

SelectConnection Spire::Styles::select(const FocusIn& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return SelectConnection(std::make_unique<FocusExecutor>(
    FocusObserver::State::FOCUS_IN, base, on_update));
}

SelectConnection Spire::Styles::select(const FocusVisible& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return SelectConnection(std::make_unique<FocusExecutor>(
    FocusObserver::State::FOCUS_VISIBLE, base, on_update));
}

SelectConnection Spire::Styles::select(const Hover& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return SelectConnection(std::make_unique<HoverExecutor>(base, on_update));
}
