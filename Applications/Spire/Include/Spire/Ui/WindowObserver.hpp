#ifndef SPIRE_WINDOW_OBSERVER_HPP
#define SPIRE_WINDOW_OBSERVER_HPP
#include <memory>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * An observer class signalling a QWidget has moved from one top-level
   * window to another.
   */
  class WindowObserver {
    public:

      /**
       * Signals that the observed QWidget has moved to a different top-level
       * window.
       * @param window The top-level window that QWidget has moved to.
       */
      using WindowSignal = Signal<void (QWidget* window)>;

      /**
       * Constructs a WindowObserver.
       * @param widget The widget to observer for top-level window changes.
       */
      explicit WindowObserver(QWidget& widget);

      ~WindowObserver();

      /** Returns the observed widget's top-level window. */
      QWidget* get_window() const;

      /** Connects a slot to the window signal. */
      boost::signals2::connection connect_window_signal(
        const WindowSignal::slot_type& slot) const;

    private:
      struct EventFilter;
      std::unique_ptr<EventFilter> m_event_filter;

      WindowObserver(const WindowObserver&) = delete;
      WindowObserver& operator =(const WindowObserver&) = delete;
  };
}

#endif
