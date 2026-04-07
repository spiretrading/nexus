#ifndef SPIRE_KEY_OBSERVER_HPP
#define SPIRE_KEY_OBSERVER_HPP
#include <functional>
#include <memory>
#include <QKeyEvent>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Monitors key presses on a QWidget and all of its children. */
  class KeyObserver {
    public:

      /**
       * Signals a key press with the option to filter it.
       * @param target The QWidget that received the event.
       * @param event The QKeyEvent representing the key press.
       * @return <code>true</code> to filter the event.
       */
      using FilteredKeyPressSignal =
        Signal<bool (QWidget& target, QKeyEvent& event)>;

      /**
       * Signals a key press but does not filter it out.
       * @param target The QWidget that received the event.
       * @param event The QKeyEvent representing the key press.
       */
      using KeyPressSignal =
        std::function<void (QWidget& target, QKeyEvent& event)>;

      /**
       * Constructs a KeyObserver.
       * @param widget The parent widget to observe.
       */
      explicit KeyObserver(QWidget& widget);

      /** Connects a slot to the key press signal. */
      boost::signals2::connection connect_filtered_key_press_signal(
        const FilteredKeyPressSignal::slot_type& slot) const;

      /** Connects a slot to the key press signal. */
      boost::signals2::connection connect_key_press_signal(
        const KeyPressSignal& slot) const;

    private:
      struct EventFilter;
      mutable FilteredKeyPressSignal m_key_press_signal;
      std::shared_ptr<EventFilter> m_filter;
      boost::signals2::scoped_connection m_filter_connection;
  };
}

#endif
