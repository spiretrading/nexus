#ifndef SPIRE_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/Spire/QtPromise.hpp"

namespace Spire {

  //! Provides a source for published order imbalances.
  class OrderImbalanceIndicatorModel {
    public:

      //! Represents the result of a subscription.
      struct SubscriptionResult {

        //! Connection for the published order imbalance callback.
        boost::signals2::connection m_connection;

        //! Promise for the requested data.
        QtPromise<std::vector<Nexus::OrderImbalance>> m_snapshot;
      };

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
      virtual SubscriptionResult subscribe(
        const boost::posix_time::ptime& start,
        const boost::posix_time::ptime& end,
        const OrderImbalanceSignal::slot_type& slot) = 0;
  };
}

#endif
