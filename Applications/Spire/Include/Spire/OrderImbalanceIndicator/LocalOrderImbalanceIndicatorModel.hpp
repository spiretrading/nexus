#ifndef SPIRE_LOCAL_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_LOCAL_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/Intervals.hpp"

namespace Spire {

  /** Implements an OrderImbalanceIndicatorModel in memory. */
  class LocalOrderImbalanceIndicatorModel :
      public OrderImbalanceIndicatorModel {
    public:

      /**
       * Inserts an OrderImbalance into the model and publishes it.
       * @param imbalance The OrderImbalance to publish.
       */
      void publish(const Nexus::OrderImbalance& imbalance);
      
      SubscriptionResult<std::vector<Nexus::OrderImbalance>>
        subscribe(const TimeInterval& interval,
          const OrderImbalanceSignal::slot_type& slot) override;
  };
}

#endif
