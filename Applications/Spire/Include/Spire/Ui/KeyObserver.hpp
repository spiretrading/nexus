#ifndef SPIRE_KEY_OBSERVER_HPP
#define SPIRE_KEY_OBSERVER_HPP
#include <memory>
#include <QKeyEvent>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Monitors key presses on a QWidget and all of its children. */
  class KeyObserver {
    public:

      /**
       * Signals a key press.
       * @param target The QWidget that received the event.
       * @param event The QKeyEvent representing the key press.
       */
      using KeyPressSignal =
        Signal<void (QWidget& target, const QKeyEvent& event)>;

      /**
       * Constructs a KeyObserver.
       * @param widget The parent widget to observe.
       */
      explicit KeyObserver(QWidget& widget);

      /** Connects a slot to the key press signal. */
      boost::signals2::connection connect_key_press_signal(
        const KeyPressSignal::slot_type& slot) const;

    private:
      struct EventFilter;
      mutable KeyPressSignal m_key_press_signal;
      std::shared_ptr<EventFilter> m_filter;
      boost::signals2::scoped_connection m_filter_connection;
  };
}

#endif
