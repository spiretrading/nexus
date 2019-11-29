#ifndef SPIRE_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include <vector>
#include <boost/signals2/connection.hpp>
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/Spire/QtPromise.hpp"

namespace Spire {

  //! Provides a source for published order imbalances.
  class OrderImbalanceIndicatorModel {
    public:

      //! Stores the result of a subscription.
      struct SubscriptionResult {

        //! Connection to the order imbalance subscription.
        boost::signals2::connection m_connection;

        //! Stores a subscription's initial snapshot.
        QtPromise<std::vector<Nexus::OrderImbalance>> m_snapshot;
      };

      //! Signals that an order imbalance has been published.
      /*!
        \param imbalance The published imbalance.
      */
      using OrderImbalanceSignal =
        Signal<void (const Nexus::OrderImbalance& imbalance)>;

      virtual ~OrderImbalanceIndicatorModel() = default;

      //! Establishes a new subscription for order imbalances published within
      //! a time range.
      /*
        \param start The start of the time range (inclusive).
        \param end The end of the time range (inclusive).
        \param slot Slot called when an imbalance is published.
      */
      virtual SubscriptionResult subscribe(boost::posix_time::ptime start,
        boost::posix_time::ptime end,
        const OrderImbalanceSignal::slot_type& slot) = 0;

    protected:
      OrderImbalanceIndicatorModel() = default;
  };
}

#endif
