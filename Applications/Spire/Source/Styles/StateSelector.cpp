#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/EnabledObserver.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/HoverObserver.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct DisabledExecutor {
    const Stylist* m_stylist;
    SelectionUpdateSignal m_on_update;
    EnabledObserver m_enabled_observer;

    DisabledExecutor(const Stylist& stylist, SelectionUpdateSignal on_update)
        : m_stylist(&stylist),
          m_on_update(std::move(on_update)),
          m_enabled_observer(m_stylist->get_widget()) {
      auto& widget = m_stylist->get_widget();
      if(!widget.isEnabled()) {
        m_on_update({m_stylist}, {});
      }
      m_enabled_observer.connect_enabled_signal(
        std::bind_front(&DisabledExecutor::on_enabled, this));
    }

    void on_enabled(bool is_enabled) {
      if(is_enabled) {
        m_on_update({}, {m_stylist});
      } else {
        m_on_update({m_stylist}, {});
      }
    }
  };

  struct FocusExecutor : QObject {
    struct Observer {
      FocusObserver m_observer;
      int m_match_count;
      scoped_connection m_match_connection;

      Observer(QWidget& widget)
        : m_observer(widget),
          m_match_count(0) {}
    };
    FocusObserver::State m_expected_state;
    Stylist* m_stylist;
    SelectionUpdateSignal m_on_update;
    std::unique_ptr<Observer> m_observer;

    FocusExecutor(FocusObserver::State state, const Stylist& base,
        const SelectionUpdateSignal& on_update)
        : m_expected_state(state),
          m_stylist(const_cast<Stylist*>(&base)),
          m_on_update(on_update) {
      auto& widget = m_stylist->get_widget();
      widget.installEventFilter(this);
      if(widget.isEnabled() && widget.isVisible()) {
        initialize_observer();
      }
    }

    void initialize_observer() {
      m_observer = std::make_unique<Observer>(m_stylist->get_widget());
      m_observer->m_observer.connect_state_signal(
        std::bind_front(&FocusExecutor::on_state, this));
      if(is_match(m_observer->m_observer.get_state())) {
        ++m_observer->m_match_count;
        m_on_update({m_stylist}, {});
      }
      auto selector = [&] () -> Selector {
        if(m_expected_state == FocusObserver::State::FOCUS_IN) {
          return FocusIn();
        } else if(m_expected_state == FocusObserver::State::FOCUS_VISIBLE) {
          return FocusVisible();
        }
        return Focus();
      }();
      m_observer->m_match_connection = scoped_connection(
        m_stylist->connect_match_signal(selector, [=] (auto is_match) {
          if(is_match) {
            ++m_observer->m_match_count;
            if(m_observer->m_match_count == 1) {
              m_on_update({m_stylist}, {});
            }
          } else {
            --m_observer->m_match_count;
            if(m_observer->m_match_count == 0) {
              m_on_update({}, {m_stylist});
            }
          }
        }));
      if(m_stylist->is_match(selector)) {
        ++m_observer->m_match_count;
        if(m_observer->m_match_count == 1) {
          m_on_update({m_stylist}, {});
        }
      }
    }

    void on_state(FocusObserver::State state) {
      if(is_match(state)) {
        ++m_observer->m_match_count;
        if(m_observer->m_match_count == 1) {
          m_on_update({m_stylist}, {});
        }
      } else if(m_observer->m_match_count > 0) {
        --m_observer->m_match_count;
        if(m_observer->m_match_count == 0) {
          m_on_update({}, {m_stylist});
        }
      }
    }

    bool is_match(FocusObserver::State state) const {
      return state == m_expected_state ||
        m_expected_state == FocusObserver::State::FOCUS &&
          is_set(state, m_expected_state);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      auto& widget = *static_cast<QWidget*>(watched);
      if(!widget.isEnabled() || !widget.isVisible()) {
        if(m_observer) {
          if(m_observer->m_match_count != 0) {
            m_on_update({}, {m_stylist});
          }
          m_observer = nullptr;
        }
      } else if(!m_observer) {
        initialize_observer();
      }
      return QObject::eventFilter(watched, event);
    }
  };

  struct HoverExecutor : QObject {
    struct Observer {
      HoverObserver m_observer;
      bool m_is_match;

      Observer(QWidget& widget)
        : m_observer(widget),
          m_is_match(
            is_set(m_observer.get_state(), HoverObserver::State::MOUSE_OVER)) {}
    };
    Stylist* m_stylist;
    SelectionUpdateSignal m_on_update;
    std::unique_ptr<Observer> m_observer;

    HoverExecutor(const Stylist& base, const SelectionUpdateSignal& on_update)
        : m_stylist(const_cast<Stylist*>(&base)),
          m_on_update(on_update) {
      auto& widget = m_stylist->get_widget();
      widget.installEventFilter(this);
      if(widget.isEnabled() && widget.isVisible()) {
        initialize_observer();
      }
    }

    void initialize_observer() {
      m_observer = std::make_unique<Observer>(m_stylist->get_widget());
      m_observer->m_observer.connect_state_signal(
        std::bind_front(&HoverExecutor::on_state, this));
      if(m_observer->m_is_match) {
        m_on_update({m_stylist}, {});
      }
    }

    void on_state(HoverObserver::State state) {
      auto is_hovered = is_set(state, HoverObserver::State::MOUSE_OVER);
      if(m_observer->m_is_match) {
        if(!is_hovered) {
          m_observer->m_is_match = false;
          m_on_update({}, {m_stylist});
        }
      } else if(is_hovered) {
        m_observer->m_is_match = true;
        m_on_update({m_stylist}, {});
      }
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      auto& widget = *static_cast<QWidget*>(watched);
      if(!widget.isEnabled() || !widget.isVisible()) {
        if(m_observer) {
          if(m_observer->m_is_match) {
            m_on_update({}, {m_stylist});
          }
          m_observer = nullptr;
        }
      } else if(!m_observer) {
        initialize_observer();
      }
      return QObject::eventFilter(watched, event);
    }
  };
}

SelectConnection Spire::Styles::select(const Disabled& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return SelectConnection(std::make_unique<DisabledExecutor>(base, on_update));
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
