#ifndef SPIRE_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"

namespace Spire {

  class OrderImbalanceIndicatorModel {
    public:

      //! Signals that an order imbalance has been published.
      /*!
        \param imbalance The published imbalance.
      */
      using OrderImbalanceSignal =
        Signal<void (const Nexus::OrderImbalance& imbalance)>;

      //! Returns all order imbalances between the given start and end times,
      //! and calls the given slot when a new imbalance is published.
      /*
        \param start Start timestamp of the loaded range.
        \param end End timestamp of the loaded range.
        \param slot Slot called when a new imbalance is published.
      */
      virtual std::tuple<boost::signals2::connection,
        QtPromise<std::vector<Nexus::OrderImbalance>>>
        subscribe(const boost::posix_time::ptime& start,
          const boost::posix_time::ptime& end,
          const OrderImbalanceSignal::slot_type& slot) = 0;

    protected:

      struct OrderImbalanceSignalConnection {
        OrderImbalanceSignal m_imbalance_signal;
        boost::posix_time::ptime m_start_time;
        boost::posix_time::ptime m_end_time;
      };
  };
}

#endif
