#ifndef SPIRE_TIME_AND_SALES_CONTROLLER_HPP
#define SPIRE_TIME_AND_SALES_CONTROLLER_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/connection.hpp>
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"

namespace Spire {

  /** Implements the application controller for the TimeAndSalesWindow. */
  class TimeAndSalesController {
    public:

      /** Signals that the window associated with this controller is closed. */
      using ClosedSignal = Signal<void ()>;

      /**
       * Constructs a TimeAndSalesController.
       * @param user_profile The user's profile.
       */
      explicit TimeAndSalesController(Beam::Ref<UserProfile> user_profile);

      /**
       * Constructs a TimeAndSalesController that already controls an existing
       * TimeAndSalesWindow.
       * @param user_profile The user's profile.
       */
      TimeAndSalesController(
        Beam::Ref<UserProfile> user_profile, TimeAndSalesWindow& window);

      ~TimeAndSalesController();

      /** Displays the TimeAndSalesWindow. */
      void open();

      /** Closes the TimeAndSalesWindow. */
      void close();

      /** Connects a slot to the ClosedSignal. */
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

    private:
      struct EventFilter;
      mutable ClosedSignal m_closed_signal;
      UserProfile* m_user_profile;
      TimeAndSalesWindow* m_window;
      std::unique_ptr<EventFilter> m_event_filter;

      TimeAndSalesController(const TimeAndSalesController&) = delete;
      TimeAndSalesController& operator =(
        const TimeAndSalesController&) = delete;
      void set_window(TimeAndSalesWindow& window);
  };
}

#endif
