#ifndef SPIRE_MOUSE_OBSERVER_HPP
#define SPIRE_MOUSE_OBSERVER_HPP
#include <functional>
#include <memory>
#include <QMouseEvent>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Monitors mouse clicks on a QWidget and all of its children. */
  class MouseObserver {
    public:

      /**
       * Signals a mouse event with the option to filter it.
       * @param target The QWidget that received the event.
       * @param event The QMouseEvent representing the mouse event.
       * @return <code>true</code> to filter the event.
       */
      using FilteredMouseSignal =
        Signal<bool (QWidget& target, QMouseEvent& event)>;

      /**
       * Signals a mouse event but does not filter it out.
       * @param target The QWidget that received the event.
       * @param event The QMouseEvent representing the event.
       */
      using MouseSignal =
        std::function<void (QWidget& target, QMouseEvent& event)>;

      /**
       * Constructs a MouseObserver.
       * @param widget The parent widget to observe.
       */
      explicit MouseObserver(QWidget& widget);

      /** Connects a slot to the mouse signal. */
      boost::signals2::connection connect_filtered_mouse_signal(
        const FilteredMouseSignal::slot_type& slot) const;

      /** Connects a slot to the mouse signal. */
      boost::signals2::connection connect_mouse_signal(
        const MouseSignal& slot) const;

    private:
      struct EventFilter;
      mutable FilteredMouseSignal m_mouse_signal;
      std::shared_ptr<EventFilter> m_filter;
      boost::signals2::scoped_connection m_filter_connection;
  };
}

#endif
