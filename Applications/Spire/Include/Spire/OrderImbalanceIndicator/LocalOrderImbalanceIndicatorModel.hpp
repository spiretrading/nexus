#ifndef SPIRE_LOCAL_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_LOCAL_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include <queue>
#include <unordered_map>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"

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

    private:
      struct Subscription {
        TimeInterval m_interval;
        OrderImbalanceSignal m_signal;
      };
      std::vector<Subscription> m_subscriptions;
      std::unordered_map<Nexus::Security, Nexus::OrderImbalance> m_imbalances;
      std::queue<Nexus::OrderImbalance> m_publish_queue;

      QtPromise<std::vector<Nexus::OrderImbalance>>
        load(const TimeInterval& interval) const;
  };
}

#endif
