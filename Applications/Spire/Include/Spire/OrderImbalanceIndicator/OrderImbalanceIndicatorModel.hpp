#ifndef SPIRE_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Spire/Spire/Intervals.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Base class used to model a data source for OrderImbalances. */
  class OrderImbalanceIndicatorModel {
    public:
  
      /**
       * Signals an OrderImbalance was published.
       * @param imbalance The published OrderImbalance.
       */
      using OrderImbalanceSignal =
        Signal<void (const Nexus::OrderImbalance& imbalance)>;
  
      virtual ~OrderImbalanceIndicatorModel() = default;
  
      /**
       * Subscribes to OrderImbalances published within the given interval.
       * @param interval The time interval to subscribe to.
       * @param slot The slot to call when an OrderImbalance is published.
       * @returns A SubscriptionResult containing the most recent OrderImbalance
       *          of each security that had an imbalance published in the given
       *          interval and a connection for the given slot.
       */
      virtual SubscriptionResult<std::vector<Nexus::OrderImbalance>>
        subscribe(const TimeInterval& interval,
          const OrderImbalanceSignal::slot_type& slot) = 0;
  
    protected:
      OrderImbalanceIndicatorModel() = default;
  };
}

#endif
