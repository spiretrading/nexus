#ifndef SPIRE_ORDER_IMBALANCE_CHART_MODEL_HPP
#define SPIRE_ORDER_IMBALANCE_CHART_MODEL_HPP
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/Spire/SubscriptionResult.hpp"
#include "Spire/Spire/Intervals.hpp"

namespace Spire {

  //! Model used to publish order imbalances for a specific security.
  class OrderImbalanceChartModel : private boost::noncopyable {
    public:

      //! Signals an order imbalance.
      /*!
        \param imbalance The published order imbalance.
      */
      using OrderImbalanceSignal = Signal<
        void (const Nexus::OrderImbalance imbalance)>;

      virtual ~OrderImbalanceChartModel() = default;

      //! Subscribes to the security's published imbalances and returns the
      //! security's order imbalances within the given time interval.
      /*
        \param time_interval The requested time interval of the initial order
                             imbalances.
        \param slot The slot to call when a new order imbalance is published.
      */
      virtual SubscriptionResult<boost::optional<Nexus::OrderImbalance>>
        subscribe_order_imbalance(const TimeInterval& time_interval,
        const OrderImbalanceSignal::slot_type& slot) = 0;

    protected:
      OrderImbalanceChartModel() = default;
  };
}

#endif
